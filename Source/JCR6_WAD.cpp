// Lic:
// JCR6/Source/JCR6_WAD.cpp
// Slyvina - JCR6 - Where's All The Data
// version: 23.03.07
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
#include <JCR6_WAD.hpp>
#include <SlyvStream.hpp>

#undef WAD_Debug

using namespace std;
using namespace Slyvina::Units;

#ifdef WAD_Debug
#define Chat(a) std::cout << "\x1b[32mWAD DEBUG>\x1b[0m\t"<<a<<"\n"
#else
#define Chat(a)
#endif
namespace Slyvina {
	namespace JCR6 {
		__WAD_Config WAD_Config{ true,false };

		static string LevelFiles[] {"THINGS", "LINEDEFS", "SIDEDEFS", "VERTEXES", "SEGS", "SSECTORS", "NODES", "SECTORS", "REJECT", "BLOCKMAP", "BEHAVIOR"};

		static bool MapEntry(string f) {
			for (auto LF : LevelFiles) if (LF == f) return true;
			return false;
		}

		static JT_Entry NE(JT_Dir D,string m,string f, int sz, int of) {
			auto ret{ make_shared<_JT_Entry>() };
			ret->Name(f);
			ret->RealSize(sz);
			ret->CompressedSize(sz);
			ret->Offset(of);
			ret->Storage("Store");
			ret->MainFile = m;
			D->_Entries[Upper(f)] = ret;
			return ret;
		}

		JT_Dir WADDir(std::string file, std::string prefix, size_t offset, size_t _size, bool skipheadercheck, bool MapToDir , bool EmptyFileIsDir ) {
			if (!FileExists(file)) { JCR6_Panic("WAD not found", file); return nullptr; }
			auto BT = ReadFile(file); if (!BT) { JCR6_Panic("WAD could not be read", file); return nullptr; }
			auto size{ _size }; if (!size) size = BT->Size();
			BT->Position(offset);
			auto mainheader = BT->ReadString(4);			
			if (!(skipheadercheck || mainheader == "IWAD" || mainheader == "PWAD")) { JCR6_Panic("File is not a WAD", file); return nullptr; }
			Chat(BT->Position());
			auto FileCount = BT->ReadInt();
			auto DirOffset = BT->ReadInt();
			BT->Position(offset + DirOffset);
			Chat("WAD " << file << "; Count " << FileCount << "; Offset " << DirOffset << "; Correction " << offset << "; FOffset " << (offset + DirOffset) << "/" << BT->Position());
			string InDir{ "" };
			string InMap{ "" };
			auto ret = make_shared<_JT_Dir>();			
			for (uint32 i = 0; i < FileCount; i++) {
				auto FOffs = BT->ReadInt();
				auto FSize = BT->ReadInt();
				char CName[9]; CName[8] = 0; for (byte j = 0; j < 8; j++) CName[j] = BT->ReadChar();
				string FName = CName; //BT->ReadString(8);
				if (InMap.size()) {
					if (MapEntry(FName)) {
						NE(ret, file, InMap + "/" + FName, FSize, FOffs + offset);
					} else {
						InMap = "";
					}
				}
				if (!InMap.size()) {// NO ELSE!!!
					if (MapToDir) {
						if (
							(FName.size() == 4 && FName[0] == 'E' && FName[1] >= 48 && FName[1] <= 57 && FName[2] == 'M' && FName[3] >= 48 && FName[3] <= 57) ||
							(FName.size() == 4 && FName[0] == 'M' && FName[1] == 'A' && FName[2] == 'P' && FName[3] >= 48 && FName[3] <= 57 && FName[4] >= 48 && FName[4] <= 57)
							) {
							InMap = FName;
						}
					}
				}
				if (!InMap.size()) {
					if (EmptyFileIsDir && FSize == 0) {
						InDir = FName + "/";
					} else {
						NE(ret, file, InDir + FName, FSize, FOffs + offset);
					}
				}

			}
			return ret;
		}


#pragma region Driver
		static bool _Recognize(std::string file) {
			bool ret{ false };
			auto BT = ReadFile(file);
			if (!BT) return false;
			if (BT->Size() > 12) {
				auto header = BT->ReadString(4);
				ret = header == "IWAD" || header == "PWAD";
			}
			BT->Close();
			return ret;
		}
		static JT_Dir _Dir(std::string file, std::string prefix) {
			return WADDir(file, prefix, 0, 0, false, WAD_Config.MapToDir, WAD_Config.EmptyFileIsDir);
		}


		void InitWAD() {
			JD_DirDriver RD;
			RD.Name = "Where's All the Data?";
			RD.Recognize = _Recognize;
			RD.Dir = _Dir;
			RegisterDirDriver(RD);
		}
#pragma endregion
	}
}