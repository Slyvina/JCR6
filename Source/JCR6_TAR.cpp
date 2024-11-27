// License:
// 	JCR6/Source/JCR6_TAR.cpp
// 	TAR Driver for JCR6
// 	version: 24.11.27
// 
// 	Copyright (C) 2024 Jeroen P. Broks
// 
// 	This software is provided 'as-is', without any express or implied
// 	warranty.  In no event will the authors be held liable for any damages
// 	arising from the use of this software.
// 
// 	Permission is granted to anyone to use this software for any purpose,
// 	including commercial applications, and to alter it and redistribute it
// 	freely, subject to the following restrictions:
// 
// 	1. The origin of this software must not be misrepresented; you must not
// 	   claim that you wrote the original software. If you use this software
// 	   in a product, an acknowledgment in the product documentation would be
// 	   appreciated but is not required.
// 	2. Altered source versions must be plainly marked as such, and must not be
// 	   misrepresented as being the original software.
// 	3. This notice may not be removed or altered from any source distribution.
// End License
#include "../Headers/JCR6_TAR.hpp"
#include <iostream>
#include <SlyvStream.hpp>
#include <SlyvSTOI.hpp>

using namespace Slyvina::Units;

#undef JCR6_TAR_DEBUG


#ifdef JCR6_TAR_DEBUG
#define Chat(a) std::cout << "\x1b[32mJCR6.TAR.DEBUG> \x1b[0m"<<a<<"\n";
#else
#define Chat(a)
#endif

namespace Slyvina {
	namespace JCR6 {
		bool TAR_Verbose{ false };
		bool TAR_SymLinking{ false };
		bool TAR_ReturnWhatIHave{ false };
		bool TAR_ThoroughCheck{ false };

		inline void Error(std::string a) { std::cout << "\x1b[91mERROR in TAR:>\x1b[0m " << a << "\n"; }

		static void Verb(std::string a) {
			Chat("Verb: " << a);
			if (TAR_Verbose) std::cout << "\x1b[93mJCR6.TAR> " << a << "\n";
		}

		/*
		-- Pre-POSIX 1
		0	100	File path and name
		100	8	File mode (octal)
		108	8	Owner's numeric user ID (octal)
		116	8	Group's numeric user ID (octal)
		124	12	File size in bytes (octal)
		136	12	Last modification time in numeric Unix time format (octal)
		148	8	Checksum for header record
		156	1	Link indicator (file type)
		157	100	Name of linked file
		-- USTAR
		0	156	(Several fields, same as in the old format)
		156	1	Type flag
		157	100	(Same field as in the old format)
		257	6	UStar indicator, "ustar", then NUL
		263	2	UStar version, "00"
		265	32	Owner user name
		297	32	Owner group name
		329	8	Device major number
		337	8	Device minor number
		345	155	Filename prefix
		*/

		struct TAR_Entry {
			char FileName[101];
			char OFileMode[9]; int FileMode;
			char OOwnNum[9]; int OwnNum;
			char OGrpNum[9]; int GrpNum;
			char OSize[13]; int Size;
			char OModTime[13]; int ModTime;
			char CheckSum[9]; 
			char LinkIndicator; 
			char LinkedFile[101];
			uint64 NO_UStar_Offset;
			char USTARINDICATOR[7]; bool USTAR;
			char USTARVersion[2];
			char OwnUserName[33];
			char OwnGroupName[33];
			char ODeviceMajor[9], ODeviceMinor[9];
			int DeviceMajor, DeviceMinor;
			char FileNamePrefix[156];
			char Junk[50]; // I need a container, but there appears to be nothing here.
		};

		static int FOct(std::string O) { return ToInt("o" + Trim(O)); }

		#define ShowOffset Chat(TrSPrintF("Offset: %8d/%8x",BT->Position(),BT->Position()));
#define GetNum8(Oct,Int,Dsc) Chat(TrSPrintF("Offset: %8d/%8x",BT->Position(),BT->Position())); BT->ReadChars(E->Oct, 8); E->Oct[8] = 0; E->Int = FOct(E->Oct); Verb(TrSPrintF(Dsc ": %d", E->Int)+" <- '"+E->Oct+"'");
#define GetNum12(Oct,Int,Dsc) Chat(TrSPrintF("Offset: %8d/%8x",BT->Position(),BT->Position())); BT->ReadChars(E->Oct, 12); E->Oct[12] = 0; E->Int = FOct(E->Oct); Verb(TrSPrintF(Dsc ": %d", E->Int)+" <- '"+E->Oct+"'");
		static void ReadTAR_Entry(InFile BT,TAR_Entry* E) {
			Chat(TrSPrintF("***** NEW ENTRY: %x(%d)", BT->Position(), BT->Position()));

			BT->ReadChars(E->FileName, 100); E->FileName[100] = 0; Verb((String)"FileName: " + E->FileName);
			//BT->ReadChars(E->OFileMode, 8); E->OFileMode[8] = 0; E->FileMode = FOct(E->OFileMode); Verb(TrSPrintF("Filemode: %d", E->FileMode));
			E->FileNamePrefix[0] = 0;
			GetNum8(OFileMode,FileMode,"File Mode");
			GetNum8(OOwnNum, OwnNum, "Owner Number");
			GetNum8(OGrpNum, GrpNum, "Group Number");			
			GetNum12(OSize, Size, "Size");
			GetNum12(OModTime, ModTime, "Last modification time");
			ShowOffset; BT->ReadChars(E->CheckSum, 8); E->CheckSum[8] = 0; Verb((String)"CheckSum: " + E->CheckSum);
			ShowOffset; E->LinkIndicator = BT->ReadChar(); Verb(TrSPrintF("Link Indicator: %02x (%03d) -> %c", E->LinkIndicator, E->LinkIndicator, E->LinkIndicator));
			ShowOffset; BT->ReadChars(E->LinkedFile, 100); E->LinkedFile[100] = 0; Verb((String)"Linked File: " + E->FileName);
			E->NO_UStar_Offset = (uint64)BT->Position(); Verb(TrSPrintF("NO USTAR offset: %x (%d)", (int)E->NO_UStar_Offset, (int)E->NO_UStar_Offset));
			ShowOffset;  BT->ReadChars(E->USTARINDICATOR, 6); E->USTAR = !strcmp(E->USTARINDICATOR, "ustar"); Verb((String)"USTAR: " + uboolstring(E->USTAR));
			if (!E->USTAR) { BT->Position(E->NO_UStar_Offset); return; } // When no USTAR, no more data is available anyway.
			ShowOffset; BT->ReadChars(E->USTARVersion, 2); Verb(TrSPrintF("USTAR Version: %c-%c", E->USTARVersion[0], E->USTARVersion[1]));
			ShowOffset; BT->ReadChars(E->OwnUserName, 32); E->OwnUserName[32] = 0; Verb((String)"Owner username: " + E->OwnUserName);
			ShowOffset; BT->ReadChars(E->OwnGroupName, 32); E->OwnGroupName[32] = 0; Verb((String)"Owner group name: " + E->OwnGroupName);
			GetNum8(ODeviceMajor, DeviceMajor, "Device Major");
			GetNum8(ODeviceMinor, DeviceMinor, "Device Minor");
			Chat(TrSPrintF("Offset: %8d/%8x", BT->Position(), BT->Position())); BT->ReadChars(E->FileNamePrefix, 155); E->FileNamePrefix[155] = 0; Verb((String)"FileNamePrefix: " + E->OwnGroupName);
			ShowOffset; Chat("12 junk bytes");  BT->ReadChars(E->Junk, 12);
		}

#define EntryError(E) Verb((String)"ERROR in processing entry: "+E); _TAR_ERROR=E; return nullptr
		static String _TAR_ERROR{ "" };
		static JT_Entry GetEntry(InFile BT,String F) {
			_TAR_ERROR = "";
			TAR_Entry TE{};
			ReadTAR_Entry(BT, &TE);
			if (!TE.FileMode) { EntryError("File mode 000 is not likely to be correct data!"); }
			if (TE.Size > BT->Size()) { EntryError("Entry size bigger than total file size."); }
			if (TE.Size + BT->Position() > BT->Size()) { EntryError(TrSPrintF("Size %d from offset %d will lead to position %d with resource size %d. I don't think so!", TE.Size, BT->Position(), BT->Size())); }
			if (TE.Size < 0) { EntryError("Size is negative."); }
			if (TE.USTAR && (TE.USTARVersion[0] != '0' || TE.USTARVersion[1] != '0')) { EntryError(TrSPrintF("USTAR version %c%c not supported", TE.USTARVersion[0], TE.USTARVersion[1])); }			
			if (Suffixed(TE.FileName, "/")) { Verb("Directories not supported"); return nullptr; }
			JT_Entry ret{ std::shared_ptr<_JT_Entry>(new _JT_Entry()) };
			ret->Name((String)TE.FileNamePrefix + TE.FileName);
			ret->Storage("Store");
			ret->RealSize(TE.Size);
			ret->CompressedSize(TE.Size);
			ret->_ConfigInt["__CHMOD"] = TE.FileMode;
			ret->Offset(BT->Position());
			ret->_ConfigInt["__TAR_TIME"] = TE.ModTime;
			ret->MainFile = F;
			if (TE.USTAR) {
				switch (TE.LinkIndicator) {
					/*
					'0' or (ASCII NUL)	Normal file
					'1'	Hard link
					'2'	Symbolic link
					'3'	Character special
					'4'	Block special
					'5'	Directory
					'6'	FIFO
					'7'	Contiguous file
					'g'	Global extended header with meta data (POSIX.1-2001)
					'x'	Extended header with metadata for the next file in the archive (POSIX.1-2001)
					'A'–'Z'	Vendor specific extensions (POSIX.1-1988)
					All other values	Reserved for future standardization
					*/
				case 0:
				case '0':
					break; // All is well, just a normal file!
				case '1':
				case '2':
					// Symlinks. Now JCR6 won't really make a difference between a hard and a softlink.
					if (TAR_SymLinking && FileExists(TE.LinkedFile)) {
						ret->MainFile = TE.LinkedFile;
						ret->CompressedSize(FileSize(TE.LinkedFile));
						ret->RealSize(FileSize(TE.LinkedFile));
						ret->Offset(0);
						break;
					}
				case '3':EntryError("Character special, not supported");
				case '4':EntryError("Block special not supported");
				case '5': Verb("USTAR Defined directory. However JCR6 doesn't support this, it's not an error"); return nullptr;
				case '6': EntryError("FIFO not supported");
				case '7': EntryError("Contiguous file, not supported. (I've been told it was deprecated in TAR anyway"); 
				case 'g': EntryError("Global extended header with meta data not supported");
				case 'x': EntryError("No support for: Extended header with metadata for the next file in the archive"); return nullptr;
				default: EntryError(TrSPrintF("Whatever link indicator '%s' means in USTAR, no support for it (yet)", TE.LinkIndicator));
				}
			} else {
				switch (TE.LinkIndicator) {
				case 0:
				case '0':
					break; // All is well, just a normal file!
				case '1':
				case '2':
					// Symlinks. Now JCR6 won't really make a difference between a hard and a softlink.
					if (TAR_SymLinking && FileExists(TE.LinkedFile)) {
						ret->MainFile = TE.LinkedFile;
						ret->CompressedSize(FileSize(TE.LinkedFile));
						ret->RealSize(FileSize(TE.LinkedFile));
						ret->Offset(0);
						break;
					}
				default: EntryError(TrSPrintF("Whatever link indicator '%c' means in non-USTAR, no support for it (yet)", TE.LinkIndicator));
				}
			}
			BT->Seek(BT->Position() + TE.Size);
			while (!BT->ReadByte()) {
				if (BT->EndOfFile()) return ret;
			}
			BT->Seek(BT->Position() - 1);
			return ret; // true return comes later!
		}

		bool RecognizeTAR(std::string file,bool easy) {
			if (!FileExists(file)) return false;
			if (FileSize(file) < 500) return false;
			if (easy) return ExtractExt(Upper(file)) == "TAR";
			auto BT{ ReadFile(file) };
			GetEntry(BT, file); // True code comes later!
			BT->Close();
			auto ret{ _TAR_ERROR != "" }; _TAR_ERROR = "";
			return ret;
		}

		JT_Dir GetTAR(std::string file) {
			if (!FileExists(file)) { JCR6_Panic("TAR: File not found", file); return nullptr; }
			if (FileSize(file) < 500) { JCR6_Panic("TAR: Requested file is below 500 bytes. Very unlikely to be a TAR file "); return nullptr; }
			JT_Dir ret{ nullptr }; Verb("Loading: " + file);
			if (!FileExists(file)) { Error("File not found:" + file);  return nullptr; }
			auto BT{ ReadFile(file) };
			JT_Entry e;
			ret = std::shared_ptr<_JT_Dir>(new _JT_Dir());
			while (BT->Position() < BT->Size()) {
				Chat("About to go for the next entry " << BT->Position() << "/" << BT->Size());
				e = GetEntry(BT, file); 
				if (_TAR_ERROR.size()) {
					BT->Close();
					if (TAR_ReturnWhatIHave) {
						std::cout << "JCR6.TAR.ERROR:\t" << _TAR_ERROR << "\n";
						return ret;
					}
					JCR6_Panic("TAR: " + _TAR_ERROR, file);
					return nullptr;
				}
				if (e) ret->_Entries[Upper(e->Name())] = e;
			}
			BT->Close();
			return ret;
		}

		static bool DrvRec(String file) { return RecognizeTAR(file, !TAR_ThoroughCheck); }
		static JT_Dir DrvDir(String File, String Prefix) {
			auto ret = GetTAR(File); if (!Prefix.size()) return ret;
			auto retp = std::shared_ptr<_JT_Dir>(new _JT_Dir());
			auto rete = retp->Entries();
			for (auto e:*rete) {
				auto newname{ Prefix + e->Name() };
				e->_ConfigString["__Entry"] = newname;
				retp->_Entries[Upper(newname)] = e;
			}
			return retp;
		}
		void InitTAR() {
			JD_DirDriver TD{};
			TD.Recognize = DrvRec;
			TD.Dir = DrvDir;
			TD.Name = "Tape Archive";
			RegisterDirDriver(TD);
		}
	}
}
