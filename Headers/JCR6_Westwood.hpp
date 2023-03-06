// Lic:
// JCR6/Headers/JCR6_Westwood.hpp
// Slyvina - JCR6 - Westwood PAK support (header)
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
#pragma once
#include "JCR6_Core.hpp"


namespace Slyvina {
	namespace JCR6 {

		/// <summary>
		/// When set to 'false', Westwood PAK files will not be recognized. This can speed things up a little or prevent conflicts due to the chance of false positives.		
		/// </summary>
		void Westwood_Enable(bool _enabled = true);


		/// <summary>
		/// Returns the last error that occurred during the attempt to scan a Westwood pack file
		/// </summary>
		std::string Westwoord_LastError();

		void Westwood_Init(bool support = true);
	}
}