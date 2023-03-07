// Lic:
// JCR6/Headers/JCR6_WAD.hpp
// Slyvina - JCR6 - Where's All The Data (header)
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
#pragma once

// WAD = Where's All the Data?
// WAD was developed for DOOM, and has been used in DOOM, Heretic, Hexen, Rise of the Triad to name a few games.
// The PAK files that Quake use can even have WAD files inside of them.
// Now WAD is a very simplistic file format, for which it was pretty easy to write a driver so JCR6 can read it.


#include "JCR6_Core.hpp"


namespace Slyvina {
	namespace JCR6 {

		struct __WAD_Config {
			bool MapToDir = true;
			bool EmptyFileIsDir = false;
		};

		/// <summary>This is a config that JCR6 uses when analysing files through the regular JCR6 routines</summary>
		extern __WAD_Config WAD_Config;


		JT_Dir WADDir(std::string file, std::string prefix, size_t offset = 0, size_t size = 0, bool skipheadercheck = false,bool MapToDir=true,bool EmptyFileIsDir=false);
		//bool IsWAD(std::string file, size_t offset = 0);

		// The two functions above are only meant to make it possible for the QuakePAK driver to read the WAD files within the QuakePAK

		/// <summary>
		/// Enables JCR6 to recognize and read WAD files. 
		/// </summary>
		void InitWAD();
	}
}