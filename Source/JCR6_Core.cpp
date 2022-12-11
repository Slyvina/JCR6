// Lic:
// JCR6/Source/JCR6_Core.cpp
// Slyvina - JCR6 - Core
// version: 22.12.11
// Copyright (C) 2022 Jeroen P. Broks
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

#include <JCR6_Core.hpp>

using namespace std;

namespace Slyvina {
	namespace JCR6 {
#pragma region ErrorCatching
		JE_Error LastError;
		JE_Error* Last() {
			return &LastError;
		}
		static void _Error(std::string Err, std::string _Main = "N/A", std::string _Entry = "N/A") {
			LastError.Error = true;
			LastError.ErrorMessage= Err;
			LastError.MainFile = _Main;
			LastError.Entry = _Entry;
			if (JCR6PANIC) JCR6PANIC(Err);
		}
		inline void _ClearError() {
			LastError.Error = false;
			LastError.ErrorMessage = "";
			LastError.MainFile = "";
			LastError.Entry = "";
		}
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
#pragma endregion
	}
}