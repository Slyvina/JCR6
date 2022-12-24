// Lic:
// JCR6/Headers/JCR6_Core.hpp
// Slyvina - JCR6 - Core (header)
// version: 22.12.17
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
#include <SlyvString.hpp>
#include <SlyvBank.hpp>

/*
* In order to make Slyvina's JCR6 work you MUST make sure the next files are included in your project
* SlyvBank.cpp
* SlyvStream.cpp
* SlyvString.cpp
* SlyvTime.cpp
* And of course their respective headers must be available!
*/


namespace Slyvina {
	namespace JCR6 {

		class _JT_Dir; typedef std::shared_ptr<_JT_Dir> JT_Dir;

#pragma region ErrorCatching
		typedef void (*JP_Panic)(std::string);

		/// <summary>
		/// This function will be called (if available) whenever an error occurs by usage of JCR
		/// </summary>
		extern JP_Panic JCR6PANIC;


		/// <summary>
		/// Throws an error to the JCR6 error handler. 
		/// This function is only supposed to be used by JCR6 itself or any drivers you may wanna create for it!
		/// </summary>
		void JCR6_Panic(std::string Msg, std::string MainFile = "N/A", std::string Entry = "N/A");

		typedef struct __JE_Error {
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
			JT_Dir (*Dir)(std::string File, std::string Prefix);
		} JD_DirDriver;
		void RegisterDirDriver(JD_DirDriver Driver);

		typedef struct {
			std::string Name; // Name of the driver. Now please note that names must be in full lower case. Caps in any letter are reserved
			int (*Compress) (char* Uncompressed, char* Compressed, int size_uncompressed,std::string Main, std::string Ent); // This function will need to do the compression and return the size of the compressed data, or -1 if something went wrong.
			bool (*Expand)(char* Compressed, char* UnCompressed, int size_compressed, int size_uncompressed, std::string Main, std::string Ent); // This function will expand. The size_compressed parameter will check if the expanded data is indeed as long as we wanted. Will return 'true' if succesful, and 'false' if failed.
		} JC_CompressDriver;
		void RegisterCompressDriver(JC_CompressDriver Driver);

		std::map<std::string, JC_CompressDriver>* GetCompDrivers();
#pragma endregion


#pragma region ReadDirectory
		//class _JT_Dir;   typedef std::shared_ptr<_JT_Dir> JT_Dir;
		class _JT_Entry; typedef std::shared_ptr<_JT_Entry> JT_Entry;
		class _JT_Block; typedef std::shared_ptr<_JT_Block> JT_Block;

		class _JT_Dir {
		private:
			char* _LastBlockBuf{ nullptr };
			std::string _LastBlock{ "" };
			inline void FlushBlock() { if (_LastBlockBuf) { delete[] _LastBlockBuf; _LastBlockBuf = nullptr; } }
		public:
			~_JT_Dir();
			int FATOffset{ 0 };
			int FATCSize{ 0 };
			int FATSize{ 0 };
			std::string FATStorage{""};
			std::string __StartMainFile;
			std::map<std::string, std::string> ConfigString{};
			std::map<std::string, bool> ConfigBool{};
			std::map<std::string, int> ConfigInt{};
			std::map<std::string, JT_Entry> _Entries{};
			std::map<std::string, std::string> Comments{};
			std::map<std::string, JT_Block> Blocks{};

			inline std::string EName(std::string E) { return Units::Upper(Units::ChReplace(E, '\\', '/')); }


			/// <summary>
			/// Checks if an entry exists within a resource
			/// </summary>
			/// <param name="Entry"></param>
			/// <returns>true if the entry is found and false if not.</returns>
			inline bool EntryExists(std::string Entry) { return _Entries.count(EName(Entry)); }

			JT_Entry Entry(std::string Ent);
			std::shared_ptr<std::vector<JT_Entry>> Entries();

			bool DirectoryExists(std::string Dir);

			VecString Directory(std::string Dir, bool allowrecursive = true);


			/// <summary>
			/// Will return the name of the dir driver to be used in order to read this specific file into JCR6
			/// </summary>
			/// <param name="File">File in question</param>
			/// <returns>The name of the driver to be used or "NONE" if unrecognized.</returns>
			static std::string Recognize(std::string File);

			/// <summary>
			/// Loads a JCR6 file, or a file for which JCR6 is configured to be able to read, and return its directory as a JT_Dir
			/// </summary>
			/// <param name="File">File in question</param>
			/// <param name="fpath">Will put all entries in the desired folder already</param>
			/// <returns></returns>
			static JT_Dir GetDir(std::string File,std::string fpath="");


			/// <summary>
			/// Patches antoher JCR6 resource into this resource.
			/// </summary>
			/// <param name="From"></param>
			/// <param name="fpath"></param>
			void Patch(JT_Dir From,std::string fpath="");
			inline void Patch(std::string From,std::string fpath="") { return Patch(GetDir(From), fpath); }



			char* GetCharBuf(std::string Entry); // Never use this directly unless you know what you doing. Also note that memory has to be allocated for this. Don't forget to release it! All in all, never use this, unless you know what you are doing.

			/// <summary>
			/// Read a JCR6 entry and return it as a Bank (look up the Slyvina Bank system for more information)
			/// </summary>
			/// <param name="_Entry"></param>
			/// <returns></returns>
			Units::Bank B(std::string _Entry, Units::Endian End = Units::Endian::Little);
			static Units::Bank B(std::string _Main, std::string _Entry, Units::Endian End=Units::Endian::Little);

			/// <summary>
			/// Load an entire JCR6 entry as a string. (Note, this routine may not take null-termination into account, where C++ likely does)
			/// </summary>
			/// <param name="_Entry">Name of the entry</param>
			/// <returns></returns>
			inline std::string GetString(std::string _Entry) { auto BUF{ B(_Entry) }; if (BUF){ return BUF->ReadString(BUF->Size()); } else return ""; }

			/// <summary>
			/// Reads an entire JCR6 entry as a string and splits it into lines (Note: <CR> will be removed from the string and the system will then take <LF> as the end of each line. This way the Windows and Unix standards should work, but other ways (like <CR> only systems) are not supported this way).
			/// </summary>
			/// <param name="_Entry"></param>
			/// <returns></returns>
			inline VecString GetLines(std::string _Entry) { auto str{ Units::StReplace(GetString(_Entry),"\r","") }; return Units::Split(str, '\n'); }


			StringMap GetStringMap(std::string _Entry);
			inline Units::Bank GetBank(std::string _Entry, Units::Endian E = Units::Endian::Little) { return B(_Entry, E); }
		};

		class _JT_Entry {
		public:
			static JT_Entry Create(std::string Name, JT_Dir parent, bool overwrite=true);
			static inline JT_Entry Create() { return std::make_shared<_JT_Entry>(); }
			std::string MainFile{""};
			std::map<std::string, std::string> _ConfigString;
			std::map<std::string, int32> _ConfigInt;
			std::map<std::string, bool> _ConfigBool;
			int32 Correction{ 0 };
			inline int32 Block() { return _ConfigInt["__Block"]; }
			int32 Offset();
			inline std::string Name() { return _ConfigString["__Entry"]; }
			inline int32 CompressedSize() { return _ConfigInt["__CSize"]; }
			inline int32 RealSize() { return _ConfigInt["__Size"]; }
			JT_Entry Copy();
		};

		class _JT_Block {
		private:
		public:
			int ID{ 0 };
			std::string MainFile;
			int Correction{ 0 };
			std::map <std::string, bool> dataBool;
			std::map <std::string, int> dataInt;
			std::map <std::string, std::string> dataString;
			inline int CompressedSize() { return dataInt["__CSize"]; }			
			inline int RealSize() {	return dataInt["__Size"]; }
			inline int Offset() { return dataInt["__Offset"]; }
			inline _JT_Block(int _ID, std::string _MF) { ID = _ID; MainFile = _MF; }
		};


		inline JT_Dir JCR6_Dir(std::string F, std::string p = "") { return _JT_Dir::GetDir(F, p); }
#pragma endregion
	}
}