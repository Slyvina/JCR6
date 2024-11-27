// License:
// 	JCR6/Headers/JCR6_TAR.hpp
// 	TAR driver for JCR6
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
#pragma once
#include "JCR6_Core.hpp"
#include <string>

namespace Slyvina {
	namespace JCR6 {

		/// <summary>
		/// When set to true, the TAR loader will be very informative about the data it processes. (handy for debug, but that's all).
		/// </summary>
		extern bool TAR_Verbose;

		/// <summary>
		/// When set to true, the TAR loader will try to make JCR6 link to files the symlinks refer to,
		/// </summary>
		extern bool TAR_SymLinking;

		/// <summary>
		/// When unsupported data is encountered, JCR6 cannot move on. If that happens, all it can do is return what is already loaded and leave the rest be.
		/// If this is set to true, that will happen, and an error will appear on the console, but JCR6 will at least not panic.
		/// </summary>
		extern bool TAR_ReturnWhatIHave;

		/// <summary>
		/// If set to true, the recognizer in the JCR6 DirDriver will try if at least one entry can be found. Otherwise it will just check if the file if suffixed with .tar
		/// </summary>
		extern bool TAR_ThoroughCheck;

		/// <summary>
		/// Tries to recognize whether the file is a TAR file or not. P
		/// NOTE: TAR has no official way to be recognized. This function can only try to get the data of the first entry and try to see if it doesn't create any errors. False positives are therefore a real possibility.
		/// </summary>
		/// <returns></returns>
		bool RecognizeTAR(std::string file,bool easy=false);

		/// <summary>
		/// Reads a Tape Archive (TAR) as it were a JCR6 file.
		/// Please note that support is limited and that TAR does support stuff that JCR6 doesn't (such entries will be skipped)
		/// </summary>
		/// <param name="file"></param>
		/// <returns></returns>
		JT_Dir GetTAR(std::string file);

		/// <summary>
		/// Initize TAR as readable by JCR6
		/// </summary>
		void InitTAR();

		
	}
}
