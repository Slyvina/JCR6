// Lic:
// JCR6/Headers/JCR6_Core.hpp
// Slyvina - JCR6 - Core (header)
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

#pragma once
#include <Slyvina.hpp>


namespace Slyvina {
	namespace JCR6 {

		class _JT_Dir; typedef std::shared_ptr<_JT_Dir> JT_Dir;

#pragma region ErrorCatching
		typedef void (*JP_Panic)(std::string);

		/// <summary>
		/// This function will be called (if available) whenever an error occurs by usage of JCR
		/// </summary>
		JP_Panic JCR6PANIC{ nullptr };

		typedef struct {
			bool Error{ false };
			std::string ErrorMessage;
			std::string MainFile;
			std::string Entry;
		} JE_Error;
		JE_Error* Last(); // Contains the data of the last error.
#pragma endregion

#pragma region Drivers_Basis

		typedef struct {
			std::string Name;
			bool (*Recognize)(std::string File);
			JT_Dir(*Dir)(std::string File);
		} JD_DirDriver;
		void RegisterDirDriver(JD_DirDriver Driver);

		typedef struct {
			std::string Name; // Name of the driver. Now please note that names must be in full lower case. Caps in any letter are reserved
			int (*Compress) (char* Uncompressed, char* Compressed, int size_uncompressed); // This function will need to do the compression and return the size of the compressed data, or -1 if something went wrong.
			bool (*Expand)(char* Compressed, char* UnCompressed, int size_compressed, int size_uncompressed); // This function will expand. The size_compressed parameter will check if the expanded data is indeed as long as we wanted. Will return 'true' if succesful, and 'false' if failed.
		} JC_CompressDriver;
		void RegisterCompressDriver(JC_CompressDriver Driver);

#pragma endregion
	}
}