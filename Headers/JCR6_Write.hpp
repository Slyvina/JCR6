// Lic:
// JCR6/Headers/JCR6_Write.hpp
// Slyvina - JCR6 - Writer (header)
// version: 22.12.14
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
#include "JCR6_Core.hpp"
#include <SlyvBank.hpp>
#include <SlyvStream.hpp>

namespace Slyvina {
	namespace JCR6 {

		SlyvClass(_JT_Create, JT_Create);
		SlyvClass(_JT_CreateStream, JT_CreateStream);
		SlyvClass(_JT_CreateBlock, JT_CreateBlock);
		SlyvClass(_JT_Import, JT_Import);

		class _JT_Import {
		public:
			std::string file, deptype, sig;
			inline _JT_Import(std::string f, std::string s, std::string d) { file = f; deptype = d; sig = s; }
		};


		class _JT_Create {
		private:
			std::string FileTableStorage{ "Store" };
			int ftoffint{ 0 };
			std::string _MainFile;
			bool closed = false;
			std::vector<JT_Import> Dependencies;
		public:
			static int MaxHashSize;
			//public QuickStream mystream;
			Units::OutFile mystream;
			std::map<uint32, JT_CreateBlock> Blocks{};
			std::map<std::string, JT_CreateStream> OpenEntries{}; //public Dictionary<TJCRCreateStream, string> OpenEntries = new Dictionary<TJCRCreateStream, string>();
			std::map<uint32, JT_CreateBlock> OpenBlocks{}; //public Dictionary<TJCRCreateBlock, uint> OpenBlocks = new Dictionary<TJCRCreateBlock, uint>();
			std::map<std::string, JT_Entry> Entries{};
			std::map<std::string, std::string> Comments{};
			JT_Entry LastAddedEntry{ nullptr };

			inline std::string MainFile() { return _MainFile; }


			/// <summary>
			/// Creates a stream for an entry you want to add to the JCR6 resource. 
			/// </summary>
			/// <remarks>JCR6 uses memory streams for this, so whatever you add to this, keep the limitations of your RAM in mind.</remarks>
			/// <returns>The entry's stream.</returns>
			/// <param name="Entry">Entry name.</param>
			/// <param name="Storage">Storage/compression algorithm.</param>
			/// <param name="Author">Author name.</param>
			/// <param name="Notes">Notes.</param>
			/// <param name="Endian">Endian setting.</param>
			JT_CreateStream nb(std::string Entry, std::string Storage = "Store", std::string Author = "", std::string Notes = "", Units::Endian _Endian = Units::Endian::Little);

			/// <summary>
			/// Creates a stream for an entry you want to add to the JCR6 resource. 
			/// </summary>
			/// <remarks>JCR6 uses memory streams for this, so whatever you add to this, keep the limitations of your RAM in mind.</remarks>
			/// <returns>The entry's stream.</returns>
			/// <param name="Entry">Entry name.</param>
			/// <param name="Storage">Storage/compression algorithm.</param>
			/// <param name="Author">Author name.</param>
			/// <param name="Notes">Notes.</param>
			/// <param name="Endian">Endian setting.</param>
			inline JT_CreateStream NewEntry(std::string Entry, std::string Storage = "Store", std::string Author = "", std::string Notes = "", Units::Endian _Endian = Units::Endian::Little) { return nb(Entry, Storage, Author, Notes, _Endian); }


			/// <summary>
			/// Saves a stringmap (Dictionary&lt;string,string&gt;&lt;/string&gt;) into a JCR6 file as an entry
			/// </summary>
			/// <param name="data">The stringmap in question</param>
			/// <param name="Entry">Entry name.</param>
			/// <param name="Storage">Storage algorith.</param>
			/// <param name="Author">Author name.</param>
			/// <param name="Notes">Notes.</param>
			void NewStringMap(StringMap data, std::string Entry, std::string Storage = "Store", std::string Author = "", std::string Notes = "");

			/// <summary>
			/// Copies a JCR6 entry exactly the way it is into the new JCR6 resource! Very important, no repacks or anything take place, the entry will be copied the way it is. If the new JCR6 resource must run on an engine that does not support the used compression method, using this function is NOT recommended! Also entries that are part of a block can NOT be copied this way (for obvious reasons).
			/// </summary>
			/// <param name="OriginalJCR"></param>
			/// <param name="OriginalEntry"></param>
			/// <param name="TargetEntry"></param>
			void JCRCopy(JT_Dir OriginalJCR, std::string OriginalEntry, std::string TargetEntry = "");

			inline void JCRCopy(std::string OJCR, std::string OriginalEntry, std::string TargetEntry = "") {
				//try {
				Last()->Error = false;
				JCRCopy(JCR6_Dir(OJCR), OriginalEntry, TargetEntry);
				//} catch (Exception Mislukt) {
				//	JCR6.Fail($".NET Exception during JCRCopy: {Mislukt.Message}", $"<OriResource> => {MainFile}", $"({OriginalEntry} => {TargetEntry}");
				//}
			}

			/// <summary>
			/// Creates an "alias" of a JCR6 entry. In JCR6 an "Alias" is just a second entry poiting to the same data as another entry. With advanced JCR6 usage, this can sometimes make your life a lot easier.
			/// </summary>
			/// <remarks>If the target already exists, JCR6 will just override the reference, but NOT the data, so that can lead to unaccesible data in your JCR6 file. Second, JCR6 is NOT able to tell which entry is the "orginal" and which is the "target". For JCR6 they are just two separate entries and it really doesn't care that all their pointer data is the same.
			/// </remarks>
			/// <param name="original">Original entry.</param>
			/// <param name="target">Target entry.</param>
			void Alias(std::string original, std::string target);


			inline void AddDependency(std::string file, std::string sig, std::string deptype) { Dependencies.push_back(std::make_shared<_JT_Import>(file, sig, deptype)); }
			inline void Import(std::string file, std::string sig = "") { AddDependency(file, sig, "IMPORT"); }
			inline void Require(std::string file, std::string sig = "") { AddDependency(file, sig, "REQUIRE"); }

			void CloseAllEntries();
			void CloseAllBlocks();

			void AddString(std::string mystring, std::string Entry, std::string Storage = "Store", std::string Author = "", std::string Notes = "");
			void AddBytes(byte* mybuffer, uint32 buffersize, std::string Entry, std::string Storage = "Store", std::string Author = "", std::string Notes = "");
			void AddFile(std::string OriginalFile, std::string Entry, std::string Storage = "Store", std::string Author = "", std::string Notes = "");

			void AddBank(Units::Bank bnk, std::string Entry, std::string Storage = "Store", std::string A = "Author", std::string Notes = "");

			inline void AddComment(std::string name, std::string comment) { Comments[name] = comment; }

			/// <summary>
			/// Closes and finalizes JCR6 file so it's ready for usage.
			/// All Streams attacked to this JCR6 creation instance will automatically be closed and added according to their settings
			/// </summary>
			void Close();

			_JT_Create(std::string OutputFile, std::string FTStorage = "Store", std::string Signature = "");

			inline ~_JT_Create() { Close(); }

			JT_CreateBlock AddBlock(std::string Storage = "Store");
		};

		class _JT_CreateStream {
		private:
			_JT_Create* _Parent;
			bool _Closed{ false };
			Units::Bank _Buf{ nullptr };
			Units::Bank _BlockStream{ nullptr };
			std::string _Storage;
			std::string _Author;
			std::string _Notes;
			std::string _Entry;

			uint32 _Block{0};

		public:			
			inline void WriteByte(byte b) { _Buf->WriteByte(b); }
			inline void WriteInt16(int16 i) { _Buf->WriteInt16(i); }
			inline void WriteInt32(int32 i) { _Buf->WriteInt32(i); }
			inline void WriteInt64(int64 i) { _Buf->WriteInt64(i); }
			inline void WriteInt(int32 i) { _Buf->WriteInt32(i); }
			inline void WriteString(std::string s, bool raw = false) { _Buf->Write(s, raw); }
			inline void WriteBytes(byte* b,uint32 sz) { _Buf->WriteBytes(b, sz); }
			inline void WriteBytes(std::vector<byte> b) { for (auto bt : b) _Buf->WriteByte(bt); }

			inline void Write(byte b) { WriteByte(b); }
			inline void Write(int16 i) { WriteInt16(i); }
			inline void Write(int32 i) { WriteInt32(i); }
			inline void Write(int64 i) { WriteInt64(i); }
			inline void Write(std::string s, bool raw = false) { WriteString(s, raw); }
			inline void Write(StringMap sm) { _Buf->WriteStringMap(sm); }

			/// <summary>
			/// Writes a line to the JCR6 entry. Please note, this function always works UNIX style (so LF only) regardless of the platform you use.
			/// </summary>
			/// <param name="l"></param>
			inline void WriteLn(std::string l) { WriteString(l, true); WriteByte(10); }
			inline void WrilteLine(std::string l) { WriteLn(l); }

			inline uint32 Block() { return _Block; }

			_JT_CreateStream(_JT_Create* theparent, std::string theentry, std::string thestorage, std::string theauthor = "", std::string thenotes = "", Units::Endian theendian = Units::Endian::Little);

			_JT_CreateStream(_JT_Create* theparent, uint32 theblock, Units::Bank thestream, std::string theentry, std::string theauthor = "", std::string thenotes = "", Units::Endian theendian = Units::Endian::Little);

			inline Units::Bank GetStream() { return _Buf; }

			inline std::string BufAsString() { return _Buf->ToString(); }

			~_JT_CreateStream();


			void Close();
		};
		inline JT_CreateStream NewCreateStream(_JT_Create* theparent, std::string theentry, std::string thestorage, std::string theauthor = "", std::string thenotes = "", Units::Endian theendian = Units::Endian::Little) {
			auto ret = std::make_shared<_JT_CreateStream>(theparent, theentry, thestorage, theauthor, thenotes, theendian);
			theparent->OpenEntries[theentry] = ret;
			return ret;
		}

		inline JT_CreateStream NewCreateStream(_JT_Create* theparent, uint32 theblock, Units::Bank thestream, std::string theentry, std::string theauthor = "", std::string thenotes = "", Units::Endian theendian = Units::Endian::Little) {			
			JT_CreateStream ret{ new _JT_CreateStream(theparent,theblock,thestream,theentry,theauthor,thenotes,theendian) };
			theparent->OpenEntries[theentry] = ret;
		}

		class _JT_CreateBlock {
			uint32 _ID{ 0 };
			bool _closed{ false };
		public:
			std::map<std::string, int32> dataint;
			std::map<std::string, bool> databool;
			std::map<std::string, std::string> datastring;
			inline uint32 ID() { return _ID; }
			_JT_Create* _Parent;
			Units::Bank _Buf;

			inline std::string Storage() { return datastring["__Storage"]; }
			inline int32 Size() { return dataint["__Size"]; }
			inline int32 CompressedSize() { return dataint["__CSize"]; }

			inline int32  Ratio() { return (int)floor(((double)CompressedSize() / (double)Size()) * (double)100); }

			// NEVER use this constructor yourself! Always use the "AddBlock" instruction in stead!
			_JT_CreateBlock(_JT_Create* _parent, std::string aStorage = "Store");

			void Close();

			inline JT_CreateStream nb(std::string Entry, std::string Author = "", std::string Notes = "", Units::Endian _End = Units::Endian::Little) {
				//return new TJCRCreateStream(parent, ID, stream, Entry, Author, Notes, Endian);
				return NewCreateStream(_Parent, _ID, _Buf, Entry, Author, Notes, _End);
			}

			inline void NewStringMap(StringMap data, std::string Entry, std::string Author = "", std::string Notes = "") {
				auto b{ nb(Entry, Author, Notes, Units::Endian::Little) };
				b->Write(data);
				b->Close();
			}

			inline void AddBytes(byte* mybuffer, int32 sz,std::string Entry, std::string Author = "", std::string Notes = "") {
				auto s = nb(Entry, Author, Notes);
				if (!s) return;
				if (mybuffer) s->WriteBytes(mybuffer,sz);
				s->Close();
			}
			inline void AddBytes(std::vector<byte>mybuffer, std::string Entry, std::string Author = "", std::string Notes = "") {
				auto s = nb(Entry, Author, Notes);
				if (!s) return;
				s->WriteBytes(mybuffer);
				s->Close();
			}

			void AddFile(std::string OriginalFile, std::string Entry, std::string Author = "", std::string Notes = "");

			void CloseAllEntries();

			inline ~_JT_CreateBlock() {
				CloseAllEntries();
				Close();
			}
		};


		inline JT_Create CreateJCR6(std::string OutputFile, std::string FTStorage = "Store", std::string Signature = "") {
			return std::make_shared<_JT_Create>(OutputFile, FTStorage, Signature);
		}
	}
}