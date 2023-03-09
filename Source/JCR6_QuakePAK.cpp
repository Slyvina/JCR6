#pragma once
#include "../Headers/JCR6_QuakePAK.hpp"

#include <SlyvStream.hpp>

using namespace Slyvina::Units;

#define Quake_Debug

#ifdef Quake_Debug
#define Chat(a) std::cout << "\x1b[33mQuake DEBUG>\x1b[0m\t"<<a<<"\n"
#else
#define Chat(a)
#endif

namespace Slyvina {
	namespace JCR6 {
		__QuakeConfig QuakeConfig{ true, {true,false} };

	

		static QHead IsQuake(InFile BT) {
			QHead Ret{ false,0,0 };
			if (BT->Size() > 12) {
				auto Header = BT->ReadString(4);
				Ret.DirOffset = BT->ReadInt();
				Ret.FileCount = BT->ReadInt();
				Ret.IsQuake = Header == "PACK" && (Ret.FileCount % 64 == 0);
				Chat("Header: " << Header << "; FileCount: " << Ret.FileCount << "(" << (Ret.FileCount % 64) << ")");
			}
			return Ret;
		}

		QHead IsQuake(std::string file) {			
			if (!FileExists(file)) {
				return QHead{ false, 0, 0 };
			}
			auto BT = ReadFile(file);
			auto R = IsQuake(BT);
			BT->Close();
			return R;
		}
		JT_Dir QuakeDir(std::string file, std::string prefix,bool mergeWAD,__WAD_Config* WADC) {
			using namespace std;
			if (!FileExists(file)) return nullptr;
			Chat("Reading Quake Pack: " << file);
			auto BT = ReadFile(file);
			auto Ret = make_shared<_JT_Dir>();
			auto QH = IsQuake(BT);
			BT->Position(QH.DirOffset);
			for (int Ak = 0; Ak < QH.FileCount; Ak += 64) {
				Chat(Ak << "/" << QH.FileCount);
				//'DebugLog "Reading entry #"+Ak
				if (BT->EndOfFile()) break;
				//E = new TJCREntry();
				//E.PVars = New StringMap ' Just has to be present to prevent crashes in viewer based software.
				//E.MainFile = QuakePAK;
				auto FN = BT->ReadString(56); 
				auto FNU = Upper(FN); Chat("Analysing Quake Entry: " << FN << " -> " << FNU);
				auto FOff = BT->ReadInt();
				auto FSiz = BT->ReadInt();
				if (ExtractExt(FNU) == "WAD" && mergeWAD) {
					Chat("Merging WAD: " << FN);
					auto WAD = WADDir(file, prefix + FN + "/", FOff, FSiz, true, (!WADC) || WADC->MapToDir, WADC && WADC->MapToDir);
					for (auto E : WAD->_Entries) Ret->_Entries[E.first] = E.second;
				} else {
					auto E = make_shared<_JT_Entry>();
					E->Offset(FOff);
					E->RealSize(FSiz);
					E->CompressedSize(E->RealSize());
					E->Storage("Store");
					E->Name(FN);
					E->MainFile = file;
					Ret->_Entries[FNU] = E;
				}
			}
			BT->Close();
			return Ret;
		}


#pragma region Driver
		static bool _Recognize(std::string file) {
			return IsQuake(file).IsQuake;
		}

		static JT_Dir _Dir(std::string file, std::string prefix) {
			return QuakeDir(file, prefix, QuakeConfig.MergeWAD, &QuakeConfig.WADConfig);
		}

		void InitQuake() {
			JD_DirDriver RD;
			RD.Name = "Quake PAK";
			RD.Recognize = _Recognize;
			RD.Dir = _Dir;
			RegisterDirDriver(RD);
		}
#pragma endregion

	}
}