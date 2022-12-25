#include <JCR6_Core.hpp>
#include <SlyvStream.hpp>
#include <SlyvDir.hpp>
#include <SlyvString.hpp>

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

	}
}