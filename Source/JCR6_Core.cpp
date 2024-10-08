// Lic:
// JCR6/Source/JCR6_Core.cpp
// Slyvina - JCR6 - Core
// version: 24.10.05
// Copyright (C) 2022, 2023, 2024 Jeroen P. Broks
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

#include <cstring>

#include <SlyvString.hpp>
#include <SlyvStream.hpp>
#include <SlyvBank.hpp>
#include <JCR6_Core.hpp>


#undef JCR6_Debug


#ifdef JCR6_Debug
#define Chat(abc) std::cout << "\x1b[32mJCR6 DEBUG>\x1b[0m " << abc << std::endl
#else
#define Chat(abc)
#endif

using namespace std;
using namespace Slyvina::Units;

namespace Slyvina {
	namespace JCR6 {

		JP_Panic JCR6PANIC{ nullptr };

#pragma region InitJCR6_Core_Header
		inline void __InitJCR6();
#pragma endregion

#pragma region ErrorCatching
		JE_Error LastError;
		void JCR6_Panic(std::string Msg, std::string _Main, std::string _Entry) {
			LastError.Error = true;
			LastError.ErrorMessage= Msg;
			LastError.MainFile = _Main;
			LastError.Entry = _Entry;
			if (JCR6PANIC) JCR6PANIC(Msg);
		}
		JE_Error* Last() {
			return &LastError;
		}
		inline void _Error(std::string Err, std::string _Main = "N/A", std::string _Entry = "N/A") { JCR6_Panic(Err, _Main, _Entry); }
		inline void _ClearError() {
			LastError.Error = false;
			LastError.ErrorMessage = "";
			LastError.MainFile = "";
			LastError.Entry = "";
		}

#define _NullError(err,main,entry) {_Error(err,main,entry); return nullptr; }
#pragma endregion

#pragma region Driver_Registration
		static map<string, JD_DirDriver> DirDrivers;
		static map<string, JC_CompressDriver> CompDrivers;
		

		void RegisterDirDriver(JD_DirDriver Driver) {
			if (DirDrivers.count(Driver.Name)) {
				_Error("Dir driver named '" + Driver.Name + "' already exists");
				return;
			}
			DirDrivers[Driver.Name] = Driver;
		}
		
		void RegisterCompressDriver(JC_CompressDriver Driver) {
			if (CompDrivers.count(Driver.Name)) {
				_Error("Compression driver named '" + Driver.Name + "' already exists");
				return;
			}
			CompDrivers[Driver.Name] = Driver;

		}
		std::map<std::string, JC_CompressDriver>* GetCompDrivers() {
			__InitJCR6();
			return &CompDrivers;
		}
#pragma endregion


#pragma region ReadDirectory
		_JT_Dir::~_JT_Dir() {
			if (_LastBlockBuf) delete[] _LastBlockBuf;
		}
		JT_Entry _JT_Dir::Entry(string Ent) {
			if (!this) {
				JCR6_Panic("Trying to get an entry from a JCR6 resource that turned out to be null");
				return nullptr;
			}
			if (!EntryExists(Ent)) {
				_Error("Entry not found!",__StartMainFile,Ent);
				return nullptr;
			}
			return _Entries[Upper(Ent)];
		}
		std::shared_ptr<std::vector<JT_Entry>> _JT_Dir::Entries() {
			auto ret{ make_shared<vector<JT_Entry>>()};
			if (!this) {
				JCR6_Panic("Trying to get the entries of a JCR6 that turned out to be null");
				return nullptr;
			}
			for (auto e : _Entries) ret->push_back(e.second);
			return ret;
		}

		
			
		

		std::string _JT_Dir::Recognize(std::string File) {
			__InitJCR6();
			for (auto d : DirDrivers)
				if (d.second.Recognize(File)) {
					Chat("Recognized '" << File << "' as " << d.first);
					return d.first;
				} else {
					Chat("NOT recognized '" << File << "' as " << d.first);
				}
			return "NONE";
		}

		JT_Dir _JT_Dir::GetDir(std::string File, std::string fpath) {
			__InitJCR6();			
			auto driv = Recognize(File);
			if (driv == "NONE") { _Error("File not recognized by JCR6", File); return nullptr; }
			Chat("File '" << File << "' has been recognized as by driver: " << driv << "!");
			auto ret = DirDrivers[driv].Dir(File,fpath);
			if (!ret) {
				_Error("Failed to get the directory of a file. with driver " + driv + ".\n" + Last()->ErrorMessage, File, fpath);
				return nullptr;
			}
			ret->__StartMainFile = File;
			return ret;
		}

		void _JT_Dir::Patch(JT_Dir From, std::string fpath) {
			if (fpath.size()) {
				fpath = ChReplace(fpath, '\\', '/');
				if (!Suffixed(fpath, "/")) fpath += "/";
			}
			if (!From) { _Error("Cannot patch from null!"); return; }
			for (auto c : From->ConfigBool) ConfigBool[c.first] = c.second;
			for (auto c : From->ConfigInt) ConfigInt[c.first] = c.second;
			for (auto c : From->ConfigString) ConfigString[c.first] = c.second;
			for (auto c : From->Comments) Comments[c.first] = c.second;
			for (auto c : From->Blocks) Blocks[c.first] = c.second;
			for (auto e : From->_Entries) {
				auto ent = e.second->Copy();
				ent->_ConfigString["__Entry"] = fpath + ent->_ConfigString["__Entry"];
				_Entries[EName(ent->Name())] = ent;
			}
		}

		char* _JT_Dir::GetCharBuf(std::string _Entry) {
			// Based on the original C++ code for JCR6, however adapted for Slyvina.
			__InitJCR6();
			//chat({ "B: Requested: ",entry });
			char* retbuf{ nullptr };
			//static JT_EntryReader nothing(1);
			//JAMJCR_Error = "Ok";
			//JT_Entry& E = Entry(entry);
			auto E = Entry(_Entry);
			//if (JAMJCR_Error != "" && JAMJCR_Error != "Ok") return;
			if (LastError.Error) return nullptr;
			if (!E) {
				JCR6_Panic("Trouble getting entry data for entry", "???", _Entry);
				return nullptr;
			}
			std::string storage{ E->_ConfigString["__Storage"] };
			if (!CompDrivers.count(storage)) {
				std::string e = "Unknown compression method: "; e += storage;
				//JamError(e);
				//return;
				_NullError(e, E->MainFile, _Entry);
			}
			std::ifstream bt;
			bt.open(E->MainFile, std::ios::binary);
			if (E->Block() == 0) {
				//FlushBlock();
				bt.seekg(E->Offset(), std::ios::beg);
				//JT_EntryReader comp{ E.CompressedSize() };
				Chat("Getting buffer of entry " << _Entry << " from main " << E->MainFile << ";  Size: " << E->RealSize() << "; Compressed: " << E->CompressedSize());
				if (E->RealSize() < 0) { _NullError("Invalid real size: " + std::to_string(E->RealSize()), E->MainFile, E->Name()); }
				if (E->CompressedSize() < 0) { _NullError("Invalid compressed size: " + std::to_string(E->CompressedSize()), E->MainFile, E->Name()); }
				auto comp = new char[E->CompressedSize()];
				retbuf = new char[E->RealSize()];
				//data.newbuf(E.RealSize());
				//std::cout << E.MainFile << "::" << E.Entry() << " (" << E.RealSize() << ")\n"; // debug only
				bt.read(comp, E->CompressedSize());//bt.read(comp.pointme(), E.CompressedSize());
				//CompDrivers[storage].Expand(comp.pointme(), data.pointme(), comp.getsize(), data.getsize());
				CompDrivers[storage].Expand(comp, retbuf,E->CompressedSize(), E->RealSize(),E->MainFile,E->Name());
				delete[]comp;
				return retbuf;
			}
			std::string BlockTag{ std::to_string(E->Block()) + ":" + E->MainFile };
			if (!Blocks.count(BlockTag)) {
				//cout << "???\n"; for (auto& DBG : Blocks) { cout << BlockTag << "!=" << DBG.first << endl; } // debug				
				_NullError("Block for entry " + E->Name() + " not found: " + BlockTag,E->MainFile,_Entry);
				//return;
			}
			//auto BLK{ BlockMap[BlockTag] };
			auto BLK{ Blocks[BlockTag] };
			if (BlockTag != _LastBlock) {
				FlushBlock();
				auto comp = new char[BLK->CompressedSize()]; //JT_EntryReader comp{ BLK.CompressedSize() };
				bt.seekg(BLK->Offset());
				bt.read(comp, BLK->CompressedSize());
				_LastBlockBuf = new char[BLK->RealSize()];
				CompDrivers[storage].Expand(comp, _LastBlockBuf, BLK->CompressedSize(), BLK->RealSize(),E->MainFile, TrSPrintF("%s (BLOCK #%d)",E->Name().c_str(), E->Block()));
				delete[] comp;
			}
			retbuf = new char[E->RealSize()]; //data.newbuf(E.RealSize());
			//auto o{ data.pointme() };
			auto RS{ E->RealSize() };
			for (auto i = 0; i < RS; ++i) retbuf[i] = _LastBlockBuf[i + E->Offset()];
			return retbuf;
		}

		Bank _JT_Dir::B(std::string _Entry,Endian End) {
			__InitJCR6();
			char* retbuf{ nullptr };
			auto E = Entry(_Entry);
			if (LastError.Error) return nullptr;
			retbuf = GetCharBuf(_Entry);
			if (LastError.Error) {
				if (retbuf) delete[] retbuf;
				return nullptr;
			}
			return TurnToBank(retbuf, E->RealSize(), End);
		}

		Bank _JT_Dir::B(std::string _Main, std::string _Entry, Endian End) {
			return GetDir(_Main)->B(_Entry, End);
		}

		StringMap _JT_Dir::GetStringMap(std::string _Entry) {
			auto ret{ NewStringMap() };
			auto bt{ B(_Entry) };
			if (LastError.Error) return nullptr;
			bt->Position(0);
			while (!bt->AtEnd()) {
				byte ctag = bt->ReadByte();
				switch (ctag) {
				case 255:
					return ret;
				case 1: {
					auto lkey = bt->ReadInt();
					std::string key{ "" }; if (lkey) key = bt->ReadString(lkey);
					auto lvalue = bt->ReadInt();
					std::string value{ "" }; if (lvalue) value = bt->ReadString(lvalue);
					(*ret)[key] = value;
				}
					  break;
				default:
					_NullError("Invalid tag in StringMap", Entry(_Entry)->MainFile, _Entry);
				}
			}
			return ret;
		}

		bool _JT_Dir::DirectoryExists(std::string Dir) {
			if (!Dir.size()) return false;
			auto CDir{ Upper(ChReplace(Dir, '\\', '/')) }; 
			if (CDir[CDir.size() - 1] != '/') CDir += '/';
			for (auto& EI : _Entries) {
				if (Prefixed(EI.first, CDir)) return true;
			}
			return false;
		}

		VecString _JT_Dir::Directory(std::string Dir, bool allowrecursive ) {
			auto ret { NewVecString() };
			auto CDir{ Upper(ChReplace(Dir, '\\', '/')) };
			auto PDir{ CDir };
			if (CDir.size() && CDir[CDir.size() - 1] != '/') CDir += '/';
			if (PDir.size() && PDir[PDir.size() - 1] == '/') Left(PDir, PDir.size() - 1);
			for (auto& EI : _Entries) {
				if (!CDir.size()) {
					if (allowrecursive || ExtractDir(EI.first) == "") ret->push_back(EI.second->Name());					
				} else if (allowrecursive && Prefixed(EI.first,CDir)) {
					ret->push_back(EI.second->Name());
				} else if (ExtractDir(EI.first) == PDir) {
					ret->push_back(EI.second->Name());
				}
			}
			return ret;
		}


		JT_Entry _JT_Entry::Create(std::string Name, JT_Dir parent, bool overwrite) {
			if (parent->EntryExists(Name) && (!overwrite)) {
				_Error("Entry already exists and 'overwrite' is set to false", parent->__StartMainFile,Name);
				return nullptr;
			}
			auto ret{ std::make_shared<_JT_Entry>() };
			parent->_Entries[parent->EName(Name)] = ret;
			ret->_ConfigString["__Entry"] = ChReplace(Name, '\\', '/');
			return ret;
		}

		int32 _JT_Entry::Offset() {
			if (Block())
				return _ConfigInt["__Offset"];
			else
				return _ConfigInt["__Offset"] + Correction;			
		}

		void _JT_Entry::Offset(int32 _offs) {
			if (Block())
				_ConfigInt["__Offset"] = _offs;
			else
				_ConfigInt["__Offset"] = _offs - Correction;
		}

		JT_Entry _JT_Entry::Copy() {
			auto ret{ Create() };
			ret->_ConfigString = _ConfigString;
			ret->_ConfigBool = _ConfigBool;
			ret->_ConfigInt = _ConfigInt;
			ret->MainFile = MainFile;
			ret->Correction = ret->Correction;
			return ret;
		}
#pragma endregion


#pragma region ReadJCR6File
#define var auto
		struct JCR6RecData {
			int Correction{ 0 };
			bool Recognized{false};
			string Err{ "" };
		};
		const char* checkheader = "JCR6\x1a";
		inline void TrueRecognize(JCR6RecData& RecD,std::string File) {
			Chat("Trying to recognize '" << File << "' as JCR6");
			RecD.Correction = 0;
			RecD.Recognized = false;			
			if (!FileExists(File)) {
				//_Error("File not found!",File);
				RecD.Err = "File not found";
				Chat("Recognize failed on file '" << File << "': File not found!");
				return;
			}
			auto ret{ false };
			auto bt{ ReadFile(File) };
			ret = bt->Size() > 10; // I don't believe a JCR6 file can even get close to that!
			auto gotheader{ bt->ReadString(strlen(checkheader)) };
			ret = ret && gotheader == checkheader;
			Chat("File '" << File << "' is " << bt->Size() << " bytes long.");
			Chat("=> CheckHeader: " << checkheader);
			Chat("=> GotHeader:   " << gotheader);
			Chat("=> Recognized: " << ret);
			if (!ret) {
				bt->Seek(bt->Size() - 4);
				if (bt->ReadString(4) == "JCR6") {
					uint16 Footer = 5;
					bt->Seek(bt->Size() - Footer); byte bits = bt->ReadByte();
					switch (bits) {
					case 8:
						Footer++;
						bt->Seek(bt->Size() - Footer);
						RecD.Correction = bt->ReadByte() + Footer;
						break;
					case 16:
						Footer+=2;
						bt->Seek(bt->Size() - Footer);
						RecD.Correction = bt->ReadUInt16() + Footer;
						break;
					case 32:
						Footer += 4;
						bt->Seek(bt->Size() - Footer);
						RecD.Correction = bt->ReadInt() + Footer;
						if (RecD.Correction < 0) { _Error("Invalid JCR6 correction value",File); return; }
						break;
					default:
						_Error(TrSPrintF("No support for %d bits in getting offset correction value", bits), File);
						return;
					}
					ret = ret && bt->ReadString(strlen(checkheader)) == checkheader;
				}
			}
			bt->Close();
			RecD.Recognized = ret;			
		}

		static bool ___JCR6Recognize(string File) {
			_ClearError();
			JCR6RecData D;
			TrueRecognize(D,File);
			return D.Recognized;
		}

		static JT_Dir ___JCR6Dir(std::string File, std::string Prefix) {
			Chat("___JCRDir(\"" << File << "\", \"" << Prefix << "\");");
			_ClearError();
			JCR6RecData D;
			TrueRecognize(D, File);
			if (!D.Recognized) {
				if (LastError.Error) return nullptr;
				_NullError("File not recognized as JCR6, yet it's being loaded by the JCR6 driver", File, "N/A");
			}
			auto BT{ ReadFile(File) }; BT->ReadString(strlen(checkheader));
			auto ret{ make_shared<_JT_Dir>() };
			int correction{ 0 }; if (D.Correction) correction = (int)BT->Size() - D.Correction;

			//cout << "TODO! Make the actual directory reader for JCR6!\n";
			// This is just the C# code, which I simply modified in order to work with Slyvina in C++
			ret->FATOffset = BT->ReadInt();
			if (ret->FATOffset <= 0) {
				BT->Close();
				_NullError("Invalid FAT offset. Maybe you are trying to read a JCR6 file that has never been properly finalized", File, "N/A");
				//return null;
			}
			byte TTag = 0;
			string Tag = "";
			do {
				TTag = BT->ReadByte();
				if (TTag != 255) { Tag = BT->ReadString(); }
				switch (TTag) {
				case 1:
					ret->ConfigString[Tag] = BT->ReadString();
					break;
				case 2:
					ret->ConfigBool[Tag] = BT->ReadByte() == 1;
					break;
				case 3:
					ret->ConfigInt[Tag] = BT->ReadInt();
					break;
				case 255:
					break;
				default:
					BT->Close();
					_NullError(TrSPrintF("Invalid config tag (%d) %s", TTag, File.c_str()), File, "N/A");
					//return null;
				}
			} while (TTag != 255);


			if (ret->ConfigBool.count("_CaseSensitive") && ret->ConfigBool["_CaseSensitive"]) {
				BT->Close();
				_NullError("Case Sensitive dir support was already deprecated and removed from JCR6 before it went to the Go language. It's only obvious that support for this was never implemented in C++ in the first place.", File, "N/A");
			}
			BT->Position(ret->FATOffset);
			bool theend = false;
			ret->FATSize = BT->ReadInt();
			ret->FATCSize = BT->ReadInt();
			ret->FATStorage = BT->ReadString();
			//  ret.Entries = map[string]TJCR6Entry{ } // Was needed in Go, but not in C#, as unlike Go, C# DOES support field assign+define
			//auto fatcbytes = BT->ReadChars(ret->FATCSize);
			if (!CompDrivers.count(ret->FATStorage)) {
#ifdef JCR6_Debug
				for (auto dd : CompDrivers) Chat("Have compression driver '" << dd.first << "'");
#endif
				_NullError(TrSPrintF("The File Table of file '%s' was packed with the '%s' algorithm, but unfortunately, there is no support for that algoritm yet", File.c_str(), ret->FATStorage.c_str()), File, "N/A");
			}
			char* fatcbytes = new char[ret->FATCSize];
			char* fatbytes = new char[ret->FATSize];
			BT->ReadChars(fatcbytes, ret->FATCSize);
			BT->Close();
			//Console.WriteLine(ret);
			//var fatbytes = JCR6.CompDrivers[ret.FATstorage].Expand(fatcbytes, ret.FATsize);
			CompDrivers[ret->FATStorage].Expand(fatcbytes, fatbytes, ret->FATCSize, ret->FATSize,File,"* File Table *");
			//bt = QuickStream.StreamFromBytes(fatbytes, QuickStream.LittleEndian); // Little Endian is the default, but I need to make sure as JCR6 REQUIRES Little Endian for its directory structures.
			auto bt{ TurnToBank(fatbytes,ret->FATSize) };
			//if (fatbytes[fatbytes.Length - 1] != 0xff) {
			if (Char2Byte(fatbytes[ret->FATSize - 1]) != 0xff) {
				Chat("fatbytes: Size:" << ret->FATSize << "; last byte: " << (int)fatbytes[ret->FATSize - 1]);
				//for (size_t i = 0; i < ret->FATSize; ++i) Chat(i << "\t >> " << (int)Char2Byte(fatbytes[i])<<"\t" <<fatbytes[i]); // VERY STRONG DEBUG ONLY!
				//for (size_t i = 0; i < ret->FATCSize; ++i) Chat(i << "\t >> " << (int)fatcbytes[i]); // VERY STRONG DEBUG ONLY!
				//				System.Diagnostics.Debug.WriteLine("WARNING! This JCR resource is probably written with the Python Prototype of JCR6 and lacks a proper ending byte.... I'll fix that");
				_NullError("FAT Ending not proper (JCR6 resource written by the first Python prototype?)", File, "N/A");
				//var fixfat = new byte[fatbytes.Length + 1];
				//fixfat[fixfat.Length - 1] = 255;
				//for (int i = 0; i < fatbytes.Length; i++) fixfat[i] = fatbytes[i];
				//fatbytes = fixfat;				
			}
			//while ((!bt.EOF) && (!theend)) {
			while (!bt->AtEnd() && (!theend)) {
				auto mtag = bt->ReadByte();
				auto ppp = bt->Position();
				switch (mtag) {
				case 0xff:
					theend = true;
					break;
				case 0x01: {
					auto tag{ bt->ReadString() };
					Chat(tag.size() << "\t" << tag);
					//var tag = bt.ReadString().ToUpper(); //strings.ToUpper(qff.ReadString(btf)); 
					//Console.WriteLine($"Read tag: '{tag}'");

					// Unfortunately strings cannot be used for 'switch' in C++, like it can be done in C#, so yeah, I gotta do this the 'hard way'
					//switch (tag) {
					//case "BLOCK": {
					if (tag == "BLOCK") {
						var ID = bt->ReadInt();
						var nb = std::make_shared<_JT_Block>(ID, File);  //= new TJCRBlock(ID, ret, file);
						var ftag = bt->ReadByte();
						nb->Correction = D.Correction;
						ret->Blocks[TrSPrintF("%d:%s", ID, File.c_str())] = nb;//ret.Blocks[$"{ID}:{file}"] = nb;
						//cout << "Block: " << TrSPrintF("%d:%s", ID, File.c_str()) << endl;
						//for (auto& DBG : ret->Blocks) cout << "Block dbg: " << DBG.first << "!\n";
						//Console.WriteLine($"Block ftag{ftag}");
						while (ftag != 255) {
							//chats("FILE TAG %d", ftag)
							switch (ftag) {
							case 1: {
								var k = bt->ReadString();
								var v = bt->ReadString();
								nb->dataString[k] = v;
								break; }
							case 2: {
								var kb = bt->ReadString();
								var vb = bt->ReadByte() > 0;
								nb->dataBool[kb] = vb;
								break; }
							case 3: {
								var ki = bt->ReadString();
								var vi = bt->ReadInt();
								nb->dataInt[ki] = vi;
								break; }
							case 255:
								break;
							default:
								// p,_:= btf.Seek(0, 1)
								//JCR6.JERROR = $"Illegal tag in BLOCK({ID}) part: {ftag} on fatpos {bt.Position}";
								//bt->Close();
								//return null;
								_NullError(TrSPrintF("Illegal tag in BLOCK definition (%d) on position (%d)", ftag, bt->Position()), File, TrSPrintF("BLOCK:%d", ID));
							}
							ftag = bt->ReadByte();
						}
					}
					//break;
					else if (tag == "FILE") { //case "FILE": {
						   //var nb = new TJCREntry{
						   //	MainFile = file
						   //};
						Chat("Reading data for FILE");
						auto nb = std::make_shared<_JT_Entry>();
						nb->MainFile = File;
						nb->Correction = D.Correction;
						/* Not needed in C#
						 * nb.Datastring = map[string]string{}
						 * nb.Dataint = map[string]int{}
						 * nb.Databool = map[string]bool{}
						 */
						var ftag = bt->ReadByte();
						while (ftag != 255) {
							//chats("FILE TAG %d", ftag)
							switch (ftag) {
							case 1: {
								var k = bt->ReadString();
								var v = bt->ReadString();
								nb->_ConfigString[k] = v;
								break; }
							case 2: {
								var kb = bt->ReadString();
								var vb = bt->ReadBoolean();
								nb->_ConfigBool[kb] = vb;
								break; }
							case 3: {
								var ki = bt->ReadString();
								var vi = bt->ReadInt();
								nb->_ConfigInt[ki] = vi;
								break; }
							case 255:
								break;
							default:
								// p,_:= btf.Seek(0, 1)
								//JCR6.JERROR = $"Illegal tag in FILE part {ftag} on fatpos {bt.Position}";
								//bt.Close();
								//return null;
								_NullError(TrSPrintF("Illegal tag in FILE part (%d) on FAT position %d", ftag, bt->Position()), File, nb->Name());
							}
							ftag = bt->ReadByte();
						}
						var centry = Upper(nb->Name());
						ret->_Entries[centry] = nb;
					}
					//	   break;
					else if (tag == "COMMENT") { //case "COMMENT":
						var commentname = bt->ReadString();
						ret->Comments[commentname] = bt->ReadString();
						//break;
					} else if (tag == "IMPORT" || tag == "REQUIRE") {
						//case "IMPORT":
						//case "REQUIRE":
							//if impdebug {
							//    fmt.Printf("%s request from %s\n", tag, file)
							//                    }
							// Now we're playing with power. Tha ability of 
							// JCR6 to automatically patch other files into 
							// one resource
						var deptag = bt->ReadByte();
						string depk;
						string depv;
						//var depm = new Dictionary<string, string>();
						map<string, string>depm{};
						while (deptag != 255) {
							depk = bt->ReadString();
							depv = bt->ReadString();
							depm[depk] = depv;
							deptag = bt->ReadByte();
						}
						var depfile = depm["File"];
						//depsig   := depm["Signature"]
						var deppatha = depm.count("AllowPath") && depm["AllowPath"] == "TRUE";
						string depcall = "";
						// var depgetpaths[2][] string
						//List<string>[] depgetpaths = new List<string>[2];
						vector<string> depgetpaths[2];
						//depgetpaths[0] = new List<string>();
						//depgetpaths[1] = new List<string>();
						var owndir = ExtractDir(File); //Path.GetDirectoryName(file);
						int deppath = 0;
						/*if impdebug{
							fmt.Printf("= Wanted file: %s\n",depfile)
							   fmt.Printf("= Allow Path:  %d\n",deppatha)
							   fmt.Printf("= ValConv:     %d\n",deppath)
							   fmt.Printf("= Prio entnum  %d\n",len(ret.Entries))
						}*/
						if (deppatha) {
							deppath = 1;
						}
						if (owndir != "") { owndir += "/"; }
						depgetpaths[0].push_back(owndir);
						depgetpaths[1].push_back(owndir);
						// TODO: JCR6: depgetpaths[1] = append(depgetpaths[1], dirry.Dirry("$AppData$/JCR6/Dependencies/") )
						//if (qstr.Left(depfile, 1) != "/" && qstr.Left(depfile, 2) != ":") {
						if (Left(depfile, 1) != "/" && Left(depfile, 2) != ":") {
							//foreach(string depdir in depgetpaths[deppath]) //for _,depdir:=range depgetpaths[deppath]
							for (string depdir : depgetpaths[deppath]) {
								if ((depcall == "") && FileExists(depdir + depfile)) {
									depcall = depdir + depfile;
								} /*else if (depcall=="" && impdebug ){
									if !qff.Exists(depdir+depfile) {
										fmt.Printf("It seems %s doesn't exist!!\n",depdir+depfile)
									}*/
							}
						} else {
							if (FileExists(depfile)) {
								depcall = depfile;
							}
						}
						if (depcall != "") {
							ret->Patch(depcall);
							//if (JCR6.JERROR != "" && tag == "REQUIRE") {//((!ret.PatchFile(depcall)) && tag=="REQUIRE"){
							if (LastError.Error) {
								//JCR6.JERROR = "Required JCR6 addon file (" + depcall + ") could not imported! Importer reported: " + JCR6.JERROR; //,fil,"N/A","JCR 6 Driver: Dir()")
								//bt.Close();
								//return null;
								_NullError(TrSPrintF("Required JCR addon file (%s) could not be imported!\nImporter repored: %s", depcall.c_str(), LastError.ErrorMessage.c_str()), File, "N/A");
							} else if (tag == "REQUIRE" && (!FileExists(depcall))) {
								//JCR6.JERROR = "Required JCR6 addon file (" + depcall + ") could not be found!"; //,fil,"N/A","JCR 6 Driver: Dir()")
								_NullError("Required JCR6 addon file (" + depcall + ") could not be found", File, "N/A");
								//bt.Close();
								//return null;
							}
						} /*else if impdebug {
							fmt.Printf("Importing %s failed!", depfile);
							fmt.Printf("Request:    %s", tag);
						}*/
						//break;
					}
					//break;
					else { //default:
					//JCR6.JERROR = $"Unknown main tag {mtag}, at file table position '{file}'::{bt.Position}/{bt.Size}";
					//JCR6.Fail($"Unknown main tag {mtag}, at file table position '{file}'::{bt.Position}/{bt.Size}", file);
					//bt.Close();
					//return null;
					_NullError(TrSPrintF("Unknown command tag '%s' at file table position '%s'::%d", tag.c_str(), File.c_str(), bt->Position()), File, "N/A");
					}
				} // case 0x01;
				break;
				default:
						_NullError(TrSPrintF("Unknown main tag '%d' at file table position '%s'::%d", mtag, File.c_str(), bt->Position()), File, "N/A");
				}
			}
			Chat("Reading '" << File << "' succesful! Returning the data");
			return ret;
		}
#pragma endregion


#pragma region Store
		static int Store_Compress(char* Uncompressed, char* Compressed, int size_uncompressed,string,string) {
			Chat("Just copying bytes, as this is 'Store'");
			for (int i = 0; i < size_uncompressed; ++i) Compressed[i] = Uncompressed[i];
			return size_uncompressed;
		}
		static bool Store_Expand(char* Compressed, char* UnCompressed, int size_compressed, int size_uncompressed,string,string) {
			if (size_compressed != size_uncompressed) {
				_Error(TrSPrintF("STORE: Internal error! Size mismatch %d!=%d", size_compressed, size_uncompressed)); 
				return false;
			}
			for (int i = 0; i < size_compressed; i++) UnCompressed[i] = Compressed[i];
			return true;
		}

#pragma endregion
#pragma region ActualInitJCR6
		inline void __InitJCR6() {
			_ClearError();
			if (!CompDrivers.count("Store")) {
				Chat("Init 'Store' compression driver!");
				CompDrivers["Store"].Compress = Store_Compress;
				CompDrivers["Store"].Expand = Store_Expand;
				CompDrivers["Store"].Name = "Store";
			}
			if (!DirDrivers.count("JCR6")) {
				Chat("Init 'JCR6' directory driver!");
				DirDrivers["JCR6"].Name = "JCR6";
				DirDrivers["JCR6"].Recognize = ___JCR6Recognize;
				DirDrivers["JCR6"].Dir = ___JCR6Dir;
			}
			//cout << "TODO: Init JCR6\n";
		}
#pragma endregion

	}





}