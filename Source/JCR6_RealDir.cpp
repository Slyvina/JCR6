// Lic:
// JCR6/Source/JCR6_RealDir.cpp
// Slyvina - JCR6 - Driver - Real Directory
// version: 22.12.25
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
#include <SlyvStream.hpp>
#include <SlyvDir.hpp>
#include <SlyvString.hpp>
#include "JCR6_RealDir.hpp"

namespace Slyvina {
	namespace JCR6 {

		bool JCR6_RealDir_AutoMerge{ true };

		static bool RDRec(std::string d) {
			return Units::DirectoryExists(d);
		}

		static JT_Dir RDDir(std::string F, std::string prefix) {
			using namespace Units;
			F = ChReplace(F, '\\', '/');
			auto Ret = std::make_shared<_JT_Dir>();
			auto FD = GetTree(F);
			for (auto& Fl : *FD) {
				auto FullFile = F; if (!Suffixed(F, "/")) F += "/"; FullFile += Fl;
				if (JCR6_RealDir_AutoMerge && _JT_Dir::Recognize(FullFile) != "NONE") {
					Ret->Patch(FullFile, Fl + "/");
				} else {
					auto Ent{ std::make_shared<_JT_Entry>() };
					Ret->_Entries[Upper(Fl)] = Ent;
					Ent->_ConfigString["__Entry"] = Fl;
					Ent->_ConfigInt["__Size"] = FileSize(FullFile);
					Ent->_ConfigInt["__CSize"] = Ret->ConfigInt["__Size"];
					Ent->_ConfigString["__Storage"] = "Store";
					Ent->MainFile = FullFile;
					if (Upper(StripAll(F)) == "DIRINFO") Ret->Comments["Dir: " + F] = FLoadString(FullFile);
				}
			}
		}

		void JCR6_InitReadDir() {
			JD_DirDriver RD;
			RD.Name = "Real Directory";
			RD.Recognize = RDRec;
			RD.Dir = RDDir;
			RegisterDirDriver(RD);
		}

		JT_Dir GetDirAsJCR(std::string path, bool AllowMerge) {
			using namespace Units;
			if (!DirectoryExists(path)) return nullptr;
			auto tm{ JCR6_RealDir_AutoMerge };
			JCR6_RealDir_AutoMerge = AllowMerge;
			auto ret = RDDir(path, "");
			JCR6_RealDir_AutoMerge = tm;
			return ret;
		}

	}
}