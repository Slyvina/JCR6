#pragma once
#include "JCR6_WAD.hpp"


namespace Slyvina {
	namespace JCR6 {

		struct __QuakeConfig {
			bool MergeWAD;
			__WAD_Config WADConfig;
		};

		struct QHead {
			bool IsQuake;
			int FileCount, DirOffset;
		};

		extern __QuakeConfig QuakeConfig;

		QHead IsQuake(std::string file);

		JT_Dir QuakeDir(std::string file, std::string prefix, bool mergeWAD = false, __WAD_Config* WADC = nullptr);


		void InitQuake();

	}
}