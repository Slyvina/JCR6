// Lic:
// JCR6/Source/JCR6_Westwood.cpp
// Slyvina - JCR6 - Westwood PAK support
// version: 23.03.06
// Copyright (C) 2023 Jeroen P. Broks
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
// claim that you wrote the original software. If you use this software
// in a product, an acknowledgment in the product documentation would be
// appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
// misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
// EndLic
#include <vector>
#include <memory>

#include <JCR6_Westwood.hpp>

#include <SlyvStream.hpp>
#include <SlyvDir.hpp>
#include <SlyvString.hpp>

// Please note that the code below was copied from my C# class, and that all I did here was adepting it to C++
// And the Slyvina framework.


using namespace std;
using namespace Slyvina::Units;

// Laziness
#define var auto

namespace Slyvina {
    namespace JCR6 {


        class _WWEnt {
        public:
            uint32
                offset,
                size;
            //internal StringBuilder FileName;
            std::string FileName;
            inline _WWEnt() { offset = 0; FileName = ""; size = 0; }
        };
        typedef shared_ptr<_WWEnt> WWEnt;
        WWEnt NewWWEnt() { return make_shared<_WWEnt>(); }


        /// <summary>
        /// Now Westwood PAK format is very extremely terrible and even beyond amateur!!
        /// Because of this, I cannot recommend to make your JCR6 based applications to support it
        /// unless it is really required or even some of the first purposes of the application
        /// you are setting up. If you want to use a kind of packing system in your game
        /// and are using JCR6 to support it Westwood PAK can better be ignored, as it's
        /// a file type that is very hard to autodetect, and false positives can easily pop up.
        /// The system supports no compression whatsoever in any way and also has no official
        /// way of being recognized.... As a matter of fact, technically JCR6 is just condemed to
        /// try to analyze the file and see if the content makes sense.... Sheeesh....
        /// </summary>
        // class JCR_WestwoodPAK:TJCRBASEDRIVER {


            /// <summary>
            /// When set to 'false', Westwood PAK files will not be recognized. This can speed things up a little or prevent conflicts due to the chance of false positives.
            /// </summary>
        static bool Enable = true;
        void Westwood_Enable(bool _enabled) { Enable = _enabled; }

        static string LastScanned = "***";
        static JT_Dir LastScannedDir = nullptr; //TJCRDIR LastScannedDir = null;
        static string LastError/*{get; private set;}*/ = "";
        string Westwoord_LastError() { return LastError; }


        static void Error(string E) {
            LastScanned = "***";
            LastScannedDir = nullptr;
            LastError = E;
            JCR6_Panic(E);
        }

        static void Scan(string file) {
            // Does the file even exist?
            if (!FileExists(file)) { Error(TrSPrintF("File not found: %s", file.c_str())); return; }
            // Prepare
            var Entries = shared_ptr<vector<WWEnt>>(new vector<WWEnt>()); //new List<WWEnt>();
            // Open
            var BT = ReadFile(file);
            WWEnt First = nullptr;
            uint32 LastOffset = 0;
            // Read the actual data
            do {
                var Ent = NewWWEnt();
                if (First == nullptr) First = Ent;
                // 4-byte file start position.
                Ent->offset = BT->ReadUInt(); //.ReadUnSignedInt();

                var Position = BT->Position();
                // Trap for version 2 and 3 PAK files.
                if (Ent->offset > BT->Size()) {
                    Error("Entry offset beyond EOF.");
                    return;
                }
                if (Ent->offset == 0) {
                    break;
                } else {
                    // Trap for version 1 PAK files.
                    if ((Position - 1) == First->offset) {
                        //Entries.Add(Ent); //FileCount = FileCount + 1
                        break;
                    } else {
                        if (Ent->offset < LastOffset) {
                            Error("Offset conflict. This cannot be a WestWood PAK");
                            return;
                        }
                        LastOffset = Ent->offset;
                        // Read the file name until we hit a null.             
                        char Char = 0;//byte Char = 0;
                        do {
                            Char = BT->ReadChar();
                            if (Char != 0) {
                                Ent->FileName += Char;//.Append((char)Char);
                                if (Char < 30 || Char > 126) { Error(TrSPrintF("Character #{%0x} is not likely used in a file name! ", (int)Char)); return; }
                            }
                        } while (Char > 0);
                        Entries->push_back(Ent); // Entries.Add(Ent); //FileCount = FileCount + 1
                    }
                }
            } while (true);
            // Reading itself is done now!
            var ResSize = (uint32)BT->Size();
            BT->Close();

            // Working with an array is easier from this point
            var EntArray = Entries; //.ToArray();
            // Was this way in C#, in C++ it doesn't matter, but it saves me a lot of headaches this way!

            // Calculating file sizes (it's really beyond me why Westwood saw fit NOT to include that essential data)
            for (uint32 FileNo = 0; FileNo < EntArray->size(); ++FileNo) {
                uint32 FileSize = 0;
                var cEnt = (*EntArray)[FileNo];
                // Get the file size.
                if (FileNo == EntArray->size() - 1) {
                    FileSize = ResSize - cEnt->offset;
                } else {
                    FileSize = (*EntArray)[FileNo + 1]->offset - cEnt->offset;
                }
                cEnt->size = FileSize;
            }

            // Now to convert all collected data to data JCR6 can understand
            var Dir = make_shared<_JT_Dir>(); //new TJCRDIR();
            for (var WE : *EntArray) {
                var E = make_shared<_JT_Entry>(); //new TJCREntry();
                E->Name(WE->FileName);
                E->RealSize(WE->size); if (E->RealSize() < 0) { Error("Invalid size data. This Westwood file may have gone beyond the limitations of JCR6"); return; } // The error is given the fact that this is a DOS format not likely to happen, but technically possible, so we must be prepared.
                E->Offset(WE->offset); if (E->Offset() < 0) { Error("Invalid offset data. This Westwood file may have gone beyond the limitations of JCR6"); return; } // The error is given the fact that this is a DOS format not likely to happen, but technically possible, so we must be prepared.
                E->Author("(?) Westwood Studios Inc. (?)");
                E->Notes("Please be aware that this file came from a Westwood PAK file. Aside from copyright the file format is so primitive that I cannot guarantee things went right");
                E->MainFile = file;
                E->Storage("Store"); // The only format PAK supports anyway, so that's easy.
                E->CompressedSize(WE->size);
                if (E->Name() != "")
                    Dir->_Entries[Upper(E->Name())] = E;
            }
            LastScanned = file;
            LastScannedDir = Dir;
        }

        static bool Recognize(string file) {
            Scan(file);
            return LastScannedDir != nullptr;
        }

        static JT_Dir Dir(string file,string ig) {
            if (file != LastScanned || LastScannedDir == nullptr)
                Scan(file);
            return LastScannedDir;
        }



        /*
        public JCR_WestwoodPAK(bool support = true) {
            name = "Westwood PAK";
            Enable = support;
            JCR6.FileDrivers[name] = this;
        }*/
        //}


        void Westwood_Init(bool support) {
            JD_DirDriver RD;
            RD.Name = "Westwood PAK";
            RD.Recognize = Recognize;
            RD.Dir = Dir;
            RegisterDirDriver(RD);
        }

    }
}