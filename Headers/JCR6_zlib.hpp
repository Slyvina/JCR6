// Lic:
// JCR6/Headers/JCR6_zlib.hpp
// Slyvina - JCR6 - zlib driver (header)
// version: 22.12.13
// Copyright (C) 2019, 2020, 2021, 2022 Jeroen P. Broks
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


// Basically this is the same driver as for the original JCR6 driver, 
// only modified to be used properly with the Slyvina version of JCR6.

#pragma once
//#ifndef JCR6_LOADEDHEADERZLIB
//#define JCR6_LOADEDHEADERZLIB

#include "JCR6_Core.hpp"
//extern "C" {
//#include <zlib.h>
#include "../3rdParty/zlib/zlib.h"
//}

namespace Slyvina {
	namespace JCR6 { void init_zlib(); }
}
inline void JCR_InitZlib() { Slyvina::JCR6::init_zlib(); }

//#endif