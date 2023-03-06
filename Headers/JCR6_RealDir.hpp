// Lic:
// JCR6/Headers/JCR6_RealDir.hpp
// Slyvina - JCR6 - Driver - Real Directory (header)
// version: 23.03.06
// Copyright (C) 2022, 2023 Jeroen P. Broks
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
#pragma once
#include <JCR6_Core.hpp>

namespace Slyvina { 
	namespace JCR6 { 
		void JCR6_InitRealDir(); 
		JT_Dir GetDirAsJCR(std::string path, bool AllowMerge = true);
		extern bool JCR6_RealDir_AutoMerge;;
	} 
}