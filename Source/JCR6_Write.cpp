// Lic:
// JCR6/Source/JCR6_Write.cpp
// Slyvina - JCR6 - Writer
// version: 22.12.13
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
#include <JCR6_Write.hpp>


#define JCR6_Debug


#ifdef JCR6_Debug
#define Chat(abc) std::cout << "\x1b[32mJCR6 DEBUG>\x1b[0m " << abc << std::endl
#else
#define Chat(abc)
#endif

// Lazyness. I was converting C# code after all!
#define var auto

using namespace Slyvina::Units;

namespace Slyvina {
	namespace JCR6 {
#pragma region JT_Create
		int _JT_Create::MaxHashSize = 100000;


		JT_CreateStream _JT_Create::nb(std::string Entry, std::string Storage, std::string Author, std::string Notes, Units::Endian _Endian) {
			if (!GetCompDrivers()->count(Storage)) { JCR6_Panic("I cannot compress with unknown storage method \"" + Storage + "\"", _MainFile, Entry); return nullptr; }
			//return std::make_shared<_JT_CreateStream>(this, Entry, Storage, Author, Notes, _Endian);
			return NewCreateStream(this, Entry, Storage, Author, Notes, _Endian);
		}

		void _JT_Create::NewStringMap(StringMap data, std::string Entry, std::string Storage, std::string Author, std::string Notes) {
			var bt = nb(Entry, Storage, Author, Notes, Endian::Little);
			if (bt == nullptr) {
				//JCR6.Fail($"Failed to create entry {Entry}, with storage method {Storage}!\t{JCR6.JCATCH}", MainFile, Entry);
				JCR6_Panic(
					TrSPrintF(
						"Failed to create entry %s with storage method %s\t%s",
						MainFile().c_str(),
						Entry.c_str(),
						Last()->ErrorMessage.c_str()
					),
					MainFile(),
					Entry
				);
				return;
			}
			//foreach(string k in data.Keys) {
			for (auto vk : *data) {
				bt->WriteByte(1);
				bt->WriteString(vk.first);
				bt->WriteString(vk.second);
			}
			bt->WriteByte(255);
			bt->Close();

		}

		void _JT_Create::JCRCopy(JT_Dir OriginalJCR, std::string OriginalEntry, std::string TargetEntry) {
			{
				//JCR6.ErrorReset();
				Last()->Error = false;
				//try {
				if (!OriginalJCR->EntryExists(OriginalEntry)) {
					JCR6_Panic("Cannot copy non-existent entry: " + OriginalEntry + "!", _MainFile, TargetEntry); return;
				}
				var oe = OriginalJCR->_Entries[Upper(OriginalEntry)]; if (oe->Block() > 0) { JCR6_Panic("Cannot copy entry that is part of a block: " + OriginalEntry, _MainFile, TargetEntry); return; }
				var bi = ReadFile(oe->MainFile); //QuickStream.ReadFile(oe.MainFile); bi.Position = oe.Offset;
				var buf = bi->ReadBytes(oe->CompressedSize());
				var ne = std::make_shared<_JT_Entry>(); //new TJCREntry();
				// Make sure all data is there, even the less common data!

				/* Old C# code kept for reference
				foreach(string k in oe.databool.Keys) ne.databool[k] = oe.databool[k];
				foreach(string k in oe.dataint.Keys) ne.dataint[k] = oe.dataint[k];
				foreach(string k in oe.datastring.Keys) ne.datastring[k] = oe.datastring[k];
				*/
				for (auto kv : oe->_ConfigBool) ne->_ConfigBool[kv.first] = kv.second;
				for (auto kv : oe->_ConfigInt) ne->_ConfigInt[kv.first] = kv.second;
				for (auto kv : oe->_ConfigString) ne->_ConfigString[kv.first] = kv.second;
				if (TargetEntry != "") ne->_ConfigString["__Entry"] = TargetEntry; //ne.Entry = TargetEntry;
				ne->_ConfigInt["__Offset"] = (int)mystream->Size(); //ne.Offset = (int)mystream.Position;
				mystream->Write(*buf, false);
				Entries[Upper(ne->Name())] = ne;
				bi->Close();
				//} catch (Exception Uitzondering) {
				//	JCR6.Fail($".NET Exception during JCRCopy: {Uitzondering.Message}", $"<OriResource> => {MainFile}", $"({OriginalEntry} => {TargetEntry}");
				//}
			}
		}

		void _JT_Create::Alias(std::string original, std::string target) {
			if (!Entries.count(Upper(original))) {
				//JCR6.JERROR = $"Cannot alias {original}. Entry not found!"; return; 
				JCR6_Panic("Cannot alias '" + original + "'. Original entry not found", _MainFile, target);
				return;
			}
			var OEntry = Entries[Upper(original)];
			//var TEntry = new TJCREntry{
			//	Entry = target,
			//	MainFile = MainFile
			//};
			var TEntry = std::make_shared<_JT_Entry>();
			//TEntry->_ConfigString["__Entry"] = target;
			TEntry->MainFile = _MainFile;
			/*
			foreach(string k in OEntry.datastring.Keys) { TEntry.datastring[k] = OEntry.datastring[k]; }
			foreach(string k in OEntry.dataint.Keys) { TEntry.dataint[k] = OEntry.dataint[k]; }
			foreach(string k in OEntry.databool.Keys) { TEntry.databool[k] = OEntry.databool[k]; }
			*/
			for (var vk : OEntry->_ConfigBool) TEntry->_ConfigBool[vk.first] = vk.second;
			for (var vk : OEntry->_ConfigInt) TEntry->_ConfigInt[vk.first] = vk.second;
			for (var vk : OEntry->_ConfigString) TEntry->_ConfigString[vk.first] = vk.second;
			TEntry->_ConfigString["__Entry"] = target; //TEntry.Entry = target; // Make sure the correct filename is there... :-/
			Entries[Upper(target)] = TEntry;

		}

		void _JT_Create::CloseAllEntries() {
			std::vector<JT_CreateStream> tl{};
			for (var s : OpenEntries) { tl.push_back(s.second); }
			for (var s : tl) { s->Close(); }
		}

		void _JT_Create::CloseAllBlocks() {
			std::vector<JT_CreateBlock> tl{};
			for (var s : OpenBlocks) { tl.push_back(s.second); }
			for (var s : tl) { s->Close(); }
		}

		void _JT_Create::AddString(std::string mystring, std::string Entry, std::string Storage, std::string Author, std::string Notes) {
			var s = nb(Entry, Storage, Author, Notes);
			if (!s) return;
			s->WriteString(mystring, true);
			s->Close();
		}

		void _JT_Create::AddBytes(byte* mybuffer, uint32 buffersize, std::string Entry, std::string Storage, std::string Author, std::string Notes) {
			Last()->Error = false;
			var s = nb(Entry, Storage, Author, Notes);
			if (!s) return;
			if (mybuffer) s->WriteBytes(mybuffer, buffersize);
			s->Close();
		}

		void _JT_Create::AddFile(std::string OriginalFile, std::string Entry, std::string Storage, std::string Author, std::string Notes) {
			Last()->Error = false;
			var rs = Units::ReadFile(OriginalFile); //= QuickStream.ReadFile(OriginalFile);
			var rsize = (int)rs->Size();
			var buf = rs->ReadBytes(rsize);
			rs->Close();
			var ws = nb(Entry, Storage, Author, Notes);
			ws->WriteBytes(*buf);
			ws->Close();
		}

		void _JT_Create::Close() {
			if (closed) return;
			Last()->Error = false; //JCR6.ErrorReset();
			CloseAllBlocks();
			CloseAllEntries();
			if (Last()->Error) return; //if (JCR6.JCATCH != null) return; // Clearly something failed here!
			//try {
			if (mystream) {
				var whereami = mystream->Position();
				mystream->Position(ftoffint); //mystream.Position = ftoffint;
				mystream->Write((int)whereami);
				mystream->Position(whereami);
				// TODO: finalizing JCR6 file
				//var ms = new MemoryStream();
				//var bt = new QuickStream(ms);
				var bt = CreateXBank();
				//foreach(string k in Comments.Keys) {
				for (var vk : Comments) {
					bt->WriteByte((byte)1);
					bt->Write("COMMENT");
					bt->Write(vk.first);
					bt->Write(vk.second);
				}
				//foreach(var Blck in Blocks) {
				for (var Blck : Blocks) {
					// Write Blocks
					bt->WriteByte(1);
					bt->WriteString("BLOCK");
					bt->WriteInt((int)Blck.first);
					//foreach(var d in Blck.Value.datastring) { bt.WriteByte(1); bt.WriteString(d.Key); bt.WriteString(d.Value); }
					//foreach(var d in Blck.Value.databool) { bt.WriteByte(2); bt.WriteString(d.Key); bt.WriteBool(d.Value); }
					//foreach(var d in Blck.Value.dataint) { bt.WriteByte(3); bt.WriteString(d.Key); bt.WriteInt(d.Value); }
					for (var vk : Blck.second->datastring) { bt->WriteByte(1); bt->Write(vk.first); bt->Write(vk.second); }
					for (var vk : Blck.second->databool) { bt->WriteByte(2); bt->Write(vk.first); bt->WriteByte(vk.second); }
					for (var vk : Blck.second->dataint) { bt->WriteByte(3); bt->Write(vk.first); bt->Write(vk.second); }
					bt->WriteByte(255);
				}
				//foreach(string k in Entries.Keys) {
				for (var vk : Entries) {
					bt->WriteByte(1);
					bt->WriteString("FILE");
					var E = vk.second;//= Entries[k];
					//foreach(string k2 in E.datastring.Keys) { bt.WriteByte(1); bt.WriteString(k2); bt.WriteString(E.datastring[k2]); }
					//foreach(string k2 in E.databool.Keys) { bt.WriteByte(2); bt.WriteString(k2); bt.WriteBool(E.databool[k2]); }
					//foreach(string k2 in E.dataint.Keys) { bt.WriteByte(3); bt.WriteString(k2); bt.WriteInt(E.dataint[k2]); }
					for (var d : E->_ConfigString) { bt->WriteByte(1); bt->Write(d.first); bt->Write(d.second); }
					for (var d : E->_ConfigBool) { bt->WriteByte(2); bt->Write(d.first); if (d.second) bt->WriteByte(1); else bt->WriteByte(0); }
					for (var d : E->_ConfigInt) { bt->WriteByte(3); bt->Write(d.first); bt->Write(d.second); }
					bt->WriteByte(255);
				}
				//foreach(TImport dependency in Dependencies) {
				for (var dependency : Dependencies) {
					bt->WriteByte(1);
					bt->WriteString(dependency->deptype);
					bt->WriteByte(1); bt->WriteString("File"); bt->WriteString(dependency->file);
					bt->WriteByte(1); bt->WriteString("Signature"); bt->WriteString(dependency->sig);
					bt->WriteByte(255);
				}
				bt->WriteByte(255);

				// TODO: "BRUTE" support file table storage
				//Console.WriteLine($"Write on {whereami}/{mystream.Position}");

				var unpacked = bt->ToChar(); //= ms.ToArray();
				var fts = FileTableStorage;
				//var packed = JCR6.CompDrivers[FileTableStorage].Compress(unpacked);
				var packed_Length = bt->Size() + (bt->Size() / 4);
				var unpacked_Length = bt->Size();
				var adrv = GetCompDrivers();
				var drv = (*adrv)[fts];
				var _packed = new char[packed_Length];
				var packed = _packed;
				packed_Length = drv.Compress(unpacked, _packed, unpacked_Length,MainFile(),"* File Table *");
				if (fts != "Store" || packed_Length >= unpacked_Length) { packed = unpacked; packed_Length = unpacked_Length; fts = "Store"; }
				//for (size_t i = 0; i < unpacked_Length; i++) std::cout << unpacked[i]; std::cout << "\n"; // DEBUG ONLY
				//bt.Close();
				mystream->Write((int)unpacked_Length);
				mystream->Write((int)packed_Length);
				mystream->Write(fts);
				mystream->WriteChars(packed, packed_Length);
				mystream->Write((int)mystream->Position());
				mystream->Write((byte)32);
				mystream->Write("JCR6", true);
				mystream->Close();
				delete[] _packed;
				delete[] unpacked;
				// Don't delete the "packed"  without the underscore. That is only a pointer copying either _packed or unpacked pointer.
			}
			closed = true;
			//} catch (Exception E) {
			//	JCR6.Fail($"<CREATE:{MainFile}>.Close(): {E.Message}", MainFile, "N/A");
			//}

		}

		_JT_Create::_JT_Create(std::string OutputFile, std::string FTStorage, std::string Signature) {
			//if (!JCR6.CompDrivers.ContainsKey(FTStorage)) { JCR6.Fail($"Storage method {FTStorage} not present!", OutputFile, "N/A"); return; }
			if (!GetCompDrivers()->count(FTStorage)) { JCR6_Panic("Storage method " + FTStorage + " not present!", OutputFile, "*File Table*"); return; }
			//			try {
			mystream = Units::WriteFile(OutputFile); //QuickStream.WriteFile(OutputFile, QuickStream.LittleEndian);
			if (!mystream) JCR6_Panic(OutputFile + " could not be created!", OutputFile, "N/A");
			FileTableStorage = FTStorage;
			mystream->Write("JCR6\x1a", true);
			ftoffint = (int)mystream->Position();
			_MainFile = OutputFile;
			mystream->Write((int32)0);
			mystream->Write((byte)1);
			mystream->Write("__Signature");
			mystream->Write(Signature);
			mystream->Write((byte)2);
			mystream->Write("__CaseSensitive");
			mystream->Write((byte)0);
			mystream->Write((byte)255);
			//			} catch (Exception E) {
			//				JCR6.Fail($"Creating JCR6 file failed: {E.Message}", OutputFile, "N/A");
						//}

		}



#pragma endregion

#pragma region CreateStream
		_JT_CreateStream::_JT_CreateStream(_JT_Create* theparent, std::string theentry, std::string thestorage, std::string theauthor, std::string thenotes, Endian theendian) {
			_Entry = theentry;
			_Storage = thestorage;
			_Author = theauthor;
			_Notes = thenotes;
			_Buf = CreateXBank(); //memstream = new MemoryStream();
			//stream = new QuickStream(memstream, Endian);
			_Parent = theparent;
			//_Parent->OpenEntries[theentry] = this;
		}


		_JT_CreateStream::_JT_CreateStream(_JT_Create* theparent, uint32 theblock, Units::Bank thestream, std::string theentry, std::string theauthor, std::string thenotes, Units::Endian theendian) {
			_Entry = theentry;
			//storage = thestorage;
			_Author = theauthor;
			_Notes = thenotes;
			//memstream = null;
			//memstream = new MemoryStream();
			//stream = new QuickStream(memstream, Endian);
			_Buf = CreateXBank();
			_BlockStream = thestream;
			_Parent = theparent;
			//parent.OpenEntries[this] = theentry;
			_Block = theblock;
			//startoffset = (uint)thestream.Size;
		}
		void _JT_CreateStream::Close() {
			if (_Closed) return;
			JT_Entry NEntry = nullptr;
			if (_Block == 0) {
				var rawbuff = _Buf->ToChar(); //memstream.ToArray();
				var hash = "Unhashed"; //if (TJCRCreate.MaxHashSize == 0 || TJCRCreate.MaxHashSize > rawbuff.Length) hash = qstr.md5(System.Text.Encoding.Default.GetString(rawbuff));
				var rawsize = _Buf->Size();
				var cmpsize = _Buf->Size() + (_Buf->Size() / 4);
				var _cmpbuff = new char[cmpsize]; //JCR6.CompDrivers[storage].Compress(rawbuff);
				var cmpbuff = _cmpbuff;
				var cdr = GetCompDrivers();
				var &cd = (*cdr)[_Storage];
				var astorage = _Storage;
				var cmpsize2 = cd.Compress(rawbuff, _cmpbuff, rawsize, _Parent->MainFile(), _Entry);
				/*
				if (cmpbuff == null) {
					JCR6.Fail("Compression buffer failed to be created!", "?", entry);
					if (stream != null) stream.Close();
					parent.OpenEntries.Remove(this);
					return;
				}
				*/
				if (!_Parent) {
					//JCR6.Fail("Parent of JCR creation stream happen to be 'null'.", "null", entry);
					delete[]_cmpbuff;
					delete[]rawbuff;
					JCR6_Panic("Parent JCR of creation stream in null", "null", _Entry);
					return;
				}
				if (!_Parent->mystream) {
					//JCR6.Fail("JCR creation impossible with non-existent stream", parent.ToString(), entry);
					delete[]_cmpbuff;
					delete[]rawbuff;
					JCR6_Panic("Stream of parent JCR of creation stream in null", _Parent->MainFile(), _Entry);
					return;
				}
				//if (2000000000 - cmpbuff.Length < parent.mystream.Size) {
				if (2000000000 - cmpsize2 < _Parent->mystream->Size()) {
					JCR6_Panic("Adding " + _Entry + " to this JCR file will exceed the limit!", _Parent->MainFile(), _Entry);
					//if (stream != null) stream.Close();

					//parent.OpenEntries.Remove(this);
					_Parent->OpenEntries.erase(_Entry);
					return;
				}
				// TODO: "BRUTE" support entry closure
				if (_Storage != "Store" && rawsize <= cmpsize2) { cmpbuff = rawbuff; astorage = "Store"; cmpsize2 = rawsize; }

				//NEntry = new TJCREntry{
				NEntry = _JT_Entry::Create();
				NEntry->_ConfigString["__Entry"] = _Entry; //Entry = entry,
				NEntry->_ConfigInt["__Size"] = rawsize;
				NEntry->_ConfigInt["__CSize"] = cmpsize2;
				NEntry->_ConfigInt["__Offset"] = _Parent->mystream->Position(); //Offset = (int)parent.mystream.Position,
				NEntry->_ConfigString["__Author"] = _Author;
				NEntry->_ConfigString["__Notes"] = _Notes;
				NEntry->_ConfigString["__Storage"] = astorage;
				//};
				NEntry->_ConfigString["__MD5HASH"] = hash;
				//parent.mystream.WriteBytes(cmpbuff);
				_Parent->mystream->WriteChars(cmpbuff, cmpsize2);
				//if (stream != null) stream.Close();
				delete[] _cmpbuff;
				delete[] rawbuff;
			} else {
				//var rawbuff = _Buf->ToChar(); //memstream.ToArray();
				var hash = "Unhashed"; //if (TJCRCreate.MaxHashSize == 0 || TJCRCreate.MaxHashSize > rawbuff.Length) hash = qstr.md5(System.Text.Encoding.Default.GetString(rawbuff));
				NEntry = _JT_Entry::Create();


				NEntry->_ConfigString["__Entry"] = _Entry; //Entry = entry,
				NEntry->_ConfigInt["__Size"] = _Buf->Size();
				NEntry->_ConfigInt["__CSize"] = 0;
				NEntry->_ConfigInt["__Offset"] = (int)_BlockStream->Position(); //Offset = (int)parent.mystream.Position,
				NEntry->_ConfigString["__Author"] = _Author;
				NEntry->_ConfigString["__Notes"] = _Notes;
				NEntry->_ConfigString["__MD5HASH"] = hash;

				NEntry->_ConfigInt["__Block"] = (int)_Block;
				_BlockStream->WriteChars(_Buf->DirectX().get());
				//blockstream.WriteBytes(rawbuff);
			}
			_Parent->LastAddedEntry = NEntry;
			NEntry->_ConfigString["__JCR6FOR"] = "Slyvina (C++)";
#ifdef SlyvWindows
			NEntry->_ConfigString["__OS"] = "Windows";
			NEntry->_ConfigInt["__CHMOD"] = 0777;
#elif SlyvMac
			NEntry->_ConfigString["__OS"] = "Mac";
#elif SlyvIOS
			NEntry->_ConfigString["__OS"] = "iOS";
#elif SlyvLinux
			NEntry->_ConfigString["__OS"] = "Linux";
#else
			NEntry->_ConfigString["__OS"] = "Unidentified";
#endif
			_Parent->Entries[Upper(NEntry->Name())] = NEntry;
			//parent.OpenEntries.Remove(this);
			_Parent->OpenEntries.erase(_Entry);
			_Closed = true;
			_Buf = nullptr;
			// stream = null;		
		}

		_JT_CreateStream::~_JT_CreateStream() { if (!_Closed) Close(); }
#pragma endregion

		_JT_CreateBlock::_JT_CreateBlock(_JT_Create* _parent, std::string aStorage) {
			Last()->Error = false;
			_Parent = _parent;
			do {} while (_Parent->Blocks.count(++_ID));
			if (!GetCompDrivers()->count(aStorage)) { JCR6_Panic("Unknown compression method: " + aStorage, _Parent->MainFile(), TrSPrintF("Block: %05d", _ID)); aStorage = "Store"; }
			datastring["__JCR6FOR"] = "Slyvina (C++)";
			//_Parent->Blocks[ID] = this;
			//parent.OpenBlocks[this] = ID;
			//memstream = new MemoryStream();
			//stream = new QuickStream(memstream);
			_Buf = CreateXBank();
			datastring["__Storage"] = aStorage;
		}

		void _JT_CreateBlock::AddFile(std::string OriginalFile, std::string Entry, std::string Author, std::string Notes) {
			var rs = ReadFile(OriginalFile);
			var buf = rs->ReadBytes((int)rs->Size());
			rs->Close();
			var ws = nb(Entry, Author, Notes);
			ws->WriteBytes(*buf);
			ws->Close();
		}

		void _JT_CreateBlock::CloseAllEntries() {
			//var tl = new List<TJCRCreateStream>();
			std::vector<JT_CreateStream> tl;
			for(var E : _Parent->OpenEntries) { if (E.second->Block() == _ID) tl.push_back(E.second); }
			for(var E : tl) E->Close();
		}

		JT_CreateBlock Slyvina::JCR6::_JT_Create::AddBlock(std::string Storage) {
			var ret = std::make_shared<_JT_CreateBlock>(this, Storage);
			OpenBlocks[ret->ID()] = ret;
			Blocks[ret->ID()] = ret;
			return ret;
		}

		void _JT_CreateBlock::Close() {
			var BLOCKTAG{ TrSPrintF("Block: %05d",_ID) };
			if (_closed) return;
			var rawbuff = _Buf->ToChar(); //memstream.ToArray();
			var rawsize = _Buf->Size();
			var cmpsize = rawsize + (rawsize / 4);
			var hash = "Unhashed"; //if (TJCRCreate.MaxHashSize == 0 || TJCRCreate.MaxHashSize > rawbuff.Length) hash = qstr.md5(System.Text.Encoding.Default.GetString(rawbuff));
			//var cmpbuff = JCR6.CompDrivers[Storage].Compress(rawbuff);
			var _cmpbuff = new char[cmpsize];
			var cmpbuff = _cmpbuff;
			var astorage = Storage();
			var drvs = GetCompDrivers();
			var drv = (*drvs)[Storage()];
			cmpsize = drv.Compress(rawbuff, _cmpbuff, rawsize, _Parent->MainFile(), BLOCKTAG);
			/*
			if (cmpbuff == null) {
				JCR6.Fail("Compression buffer failed to be created!", "?", $"Block {ID}");
				if (stream != null) stream.Close();
				parent.OpenBlocks.Remove(this);
				return;
			}
			*/
			if (!_Parent) {
				JCR6_Panic("Parent of JCR creation stream happen to be 'null'.", "null", BLOCKTAG);
				delete[] rawbuff;
				delete[] _cmpbuff;
				return;
			}
			if (!_Parent->mystream) {
				JCR6_Panic("JCR creation impossible with non-existent stream", _Parent->MainFile(), BLOCKTAG);
				delete[] rawbuff;
				delete[] _cmpbuff;
				return;
			}
			//if (2000000000 - cmpbuff.Length < parent.mystream.Size) {
			if (2000000000 - cmpsize < _Parent->mystream->Size()) {
				JCR6_Panic(TrSPrintF("Adding block %04d to this JCR file will exceed the limit!", _ID), _Parent->MainFile(), BLOCKTAG);
				//if (stream != null) stream.Close();
				//parent.OpenBlocks.Remove(this);
				_Parent->OpenBlocks.erase(_ID);
				delete[] rawbuff;
				delete[] _cmpbuff;
				return;
			}
			// TODO: "BRUTE" support entry closure
			if (Storage() != "Store" && rawsize <= cmpsize) { cmpbuff = rawbuff; astorage = "Store"; cmpsize = rawsize; }

			dataint["__Size"] = rawsize;
			dataint["__CSize"] = cmpsize;
			dataint["__Offset"] = (int)_Parent->mystream->Position();
			datastring["__MD5HASH"] = hash;
			//parent.mystream.WriteBytes(cmpbuff);
			_Parent->mystream->WriteChars(cmpbuff, cmpsize);
			//if (stream != null) stream.Close();
			_Parent->OpenBlocks.erase(_ID); //parent.OpenBlocks.Remove(this);
			_closed = true;
			//memstream = null;
			//stream = null;
			_Buf = nullptr;
			delete[] rawbuff;
			delete[] _cmpbuff;
			// Console.WriteLine($"Closed = {closed}"); // debug
		}
	}
}