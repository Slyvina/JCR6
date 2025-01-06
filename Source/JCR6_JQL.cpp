// License:
// 	JCR6/Source/JCR6_JQL.cpp
// 	JCR Quick Link
// 	version: 24.12.31
// 
// 	Copyright (C) 2023, 2024 Jeroen P. Broks
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

/********************************************************
 * I will admit that this will look UGLY!
 * I just copied in my C# code an adapted that to C++
 ********************************************************/

#undef DEBUG_JQL


//using System;
//using System.IO;
//using System.Text;
//using TrickyUnits;

#include <string.h>
#include <string>
#include <memory>

#include <JCR6_Core.hpp>

#include <SlyvDir.hpp>
#include <SlyvString.hpp>
#include <SlyvStream.hpp>
#include <SlyvBank.hpp>

#define var auto
#define _new(t,p) std::shared_ptr<t>(new t(p))

#define ccase(a)  else if (c->commando==a) 
#define ccase2(a,b) else if (c->commando==a || c->commando==b)


#ifdef DEBUG_JQL
#include <iostream>
#define Chat(abc) std::cout<<"\x1b[32mJQL Debug>\x1b[0m "<<abc<<std::endl
#else
#define Chat(abc)
#endif

using namespace std;
using namespace Slyvina::Units;


namespace Slyvina { //namespace UseJCR6 {
	namespace JCR6 {

		bool JQL_VerboseNonCriticalErrors{ false };

#pragma region JQL
		//class JCR_QuickLink :TJCRBASEDRIVER {

		class _QP {
		public:
			static std::map<String, String>* qpvars;
			std::string commando{};
			std::string parameter{};
			_QP(std::string p) {
				var i{ FindFirst(p,':') }; //= p.IndexOf(':');
				if (i < 0) {
					commando = Upper(p); //p.ToUpper();
					parameter = "";
				} else {
					commando = Upper(p.substr(0, i));
					parameter = p.substr(i + 1);
					if (qpvars) {
						try {
							for (auto v : (*qpvars)) {
								parameter = StReplace(parameter, "$" + v.first + "$", v.second);
							}
						} catch (runtime_error e) {
							if (JQL_VerboseNonCriticalErrors) cout << "ERROR: " << e.what() << " << " << p << "\n";
						}
					}
				}
			}
		};
		std::map<String, String>* _QP::qpvars{ nullptr };
		typedef shared_ptr<_QP> QP;


		//    string RL(QuickStream BT, bool trim = true) {
		string RL(InFile BT, bool trim = true) {
			string r; //var r = new StringBuilder();
			char b = 0;
			while (true) {
				if (BT->EndOfFile()) break; //if (BT.EOF) break;
				b = BT->ReadChar(); //BT.ReadByte();
				if (b == 10) break;
				if (b != 13) r += b;//r.Append((char)b);
			}
			if (trim) return Trim(r);//r.ToString().Trim();
			return r; // .ToString();
		}

		QP RQP(InFile BT) { return make_shared<_QP>(RL(BT)); }

		bool JQL_Recognize(string file) { //public override bool Recognize(string file) {
			Chat("Trying to recognize " << file);
			InFile BT{ nullptr }; //QuickStream BT = null;
			//try {
				//Console.WriteLine($"JQL Recognize {File.Exists(file)}");
			if (!FileExists(file)) {
				Chat("File not found");
				return false;
			}
			Chat("Reading");
			BT = ReadFile(file); //QuickStream.ReadFile(file);
			string s;
			do {
				Chat("ReadLine");
				s = RL(BT);
				//Console.WriteLine($"_{s}_");
				Chat("Got> " << s);
				if (s != "" && (!Prefixed(s, "#"))) return s == "JQL";
			} while (!BT->EndOfFile());
			return false;
			//} finally {
				//if (BT != nullptr) BT->Close();
			//}
		}

		JT_Dir JQL_Dir(string file, string fpath) { //public override TJCRDIR Dir(string file) {
			InFile BT = nullptr; //QuickStream BT = null;
			map<string, JT_Dir> MapFrom{};
			JT_Dir From{ nullptr };
			try {
				BT = ReadFile(file);
				var ret = make_shared<_JT_Dir>(); //= new TJCRDIR();
				string s{ "" };
				do {
					if (BT->EndOfFile()) throw runtime_error("JQL heading not found"); //throw new Exception("JQL heading not found!");
					s = RL(BT);
				} while (s == "" || Prefixed(s, "#"));
				if (s != "JQL") throw runtime_error("JQL not properly headed!");
				var onplatform{ Upper(Platform()) };
				var optional = true;
				var author = string("");
				var notes = string("");
				std::map<String, String> vars{ {"PLATFORM",Platform()} };
				_QP::qpvars = &vars;
				vars["PWD"] = CurrentDir();
				string platform{ "ALL" };
				while (!BT->EndOfFile()) {
					_QP::qpvars = &vars;
					s = RL(BT);
					var c = _new(_QP, s);					
					if (c->commando == "PLATFORM") {
						platform = Upper(c->parameter == "" ? "ALL" : c->parameter);
					}
					if (s != "" && (!Prefixed(s, "#")) && (platform=="ALL" || platform==onplatform)) {
						//switch (c->commando) {
						if (false) {}
						ccase2("VAR", "LET") {
							auto p{ IndexOf(c->parameter,'=') };
							if (p < 0) { throw runtime_error("LET syntax error >> " + s); }
							auto key{ Trim(Upper(c->parameter.substr(0,p))) };
							auto val{ c->parameter.substr(p + 1) };
							vars[key] = val;
						}
						ccase("KILL") vars.erase(Trim(Upper(c->parameter)));
						ccase2("REQUIRED", "REQ") {
							optional = false;
							//break;
						}
						ccase2("OPTIONAL", "OPT") {
							optional = true;
							//break;
						}
						ccase("PATCH") {
							var to = FindFirst(c->parameter, '>'); //c.parameter.IndexOf('>');
							if (to < 0) {
								var p = JCR6_Dir(c->parameter); //JCR6.Dir(c.parameter);
								if (p == nullptr) {
									if (optional) goto einde; //break;
									throw runtime_error(TrSPrintF("Patch error %s", Last()->ErrorMessage.c_str())); //new Exception($"Patch error {JCR6.JERROR}");
								}
								ret->Patch(p, fpath);
								_QP::qpvars = &vars;
							} else {
								var rw = ChReplace(Trim(c->parameter.substr(0, to)), '\\', '/'); //c.parameter.Substring(0, to).Trim().Replace("\\", "/");
								var tg = ChReplace(Trim(c->parameter.substr(to + 1)), '\\', '/'); //c.parameter.Substring(to + 1).Trim().Replace("\\", "/");
								var p = JCR6_Dir(rw);
								if (p == nullptr) {
									if (optional) goto einde;
									//throw new Exception($"Patch error {JCR6.JERROR}");
									throw runtime_error(TrSPrintF("Patch error %s", Last()->ErrorMessage.c_str())); //new Exception($"Patch error {JCR6.JERROR}");
								}
								ret->Patch(p, fpath + tg);
								_QP::qpvars = &vars;
							}
							//break;
						}
						ccase2("AUTHOR", "AUT") {
							author = c->parameter;
							//break;
						} 
						ccase2("NOTES", "NTS") {
							notes = c->parameter;
							//break;
						}
						ccase("ALIAS") {
							var p = FindFirst(c->parameter, '>'); 
							if (p < 0) {
								std::cout << "\x1b[31mJQL ERROR! \x1b[96mSyntax error: \x1b[92m" << s << "\x1b[0m\n";
							} else {
								var rw = c->parameter.substr(0, p);
								var tg = c->parameter.substr(p + 1);
								if (rw == "") throw runtime_error("ALIAS no original");
								if (tg == "") throw runtime_error("ALIAS no target");
								if (!ret->EntryExists(rw)) throw runtime_error("Original for alias doesn't exist!");
								var
									ori{ ret->Entry(rw) },
									tar{  make_shared<_JT_Entry>() };
								tar->MainFile = ori->MainFile;
								for (auto ai : ori->_ConfigString) tar->_ConfigString[ai.first] = ai.second;
								for (auto ai : ori->_ConfigBool) tar->_ConfigBool[ai.first] = ai.second;
								for (auto ai : ori->_ConfigInt) tar->_ConfigInt[ai.first] = ai.second;
								tar->_ConfigString["__Entry"] = tg;
								ret->_Entries[Upper(tar->Name())] = tar;
							}
						}
						ccase("RAW") {
							var p = FindFirst(c->parameter, '>');
							var rw = ChReplace(c->parameter, '\\', '/');
							var tg = rw;
							if (p >= 0) {
								//rw = c.parameter.Substring(0, p).Trim().Replace("\\", "/");
								//tg = c.parameter.Substring(p + 1).Trim().Replace("\\", "/");
								rw = ChReplace(Trim(c->parameter.substr(0, p)), '\\', '/'); //c.parameter.Substring(0, to).Trim().Replace("\\", "/");
								tg = ChReplace(Trim(c->parameter.substr(p + 1)), '\\', '/'); //c.parameter.Substring(to + 1).Trim().Replace("\\", "/");

							}
							if (tg.size() > 1 && tg[1] == ':') tg = tg.substr(2);
							while (tg[1] == '/') tg = tg.substr(1);
							if (rw == "") throw runtime_error("RAW no original");
							if (tg == "") throw runtime_error("RAW no target");
							if (!FileExists(rw)) {
								if (optional) goto einde; //break; // break would end the while loop. C++ does not support switch on strings which is quite a bummer
								throw runtime_error(TrSPrintF("Required raw file \"%s\" doesn't exist!", rw.c_str()));
							}
							var e = make_shared<_JT_Entry>(); //= new TJCREntry();
							e->_ConfigString["__Entry"] = fpath + tg; //e.Entry = tg;
							e->MainFile = rw;
							e->_ConfigString["__Storage"] = "Store";
							e->_ConfigInt["__Offset"] = 0;
							e->_ConfigInt["__Size"] = FileSize(rw); //(int)new FileInfo(rw).Length;
							e->_ConfigInt["__CSize"] = e->RealSize();
							e->_ConfigString["__Notes"] = notes;
							e->_ConfigString["__Author"] = author;
							ret->_Entries[Upper(e->Name())] = e;
							//break;
						}
						ccase("RAWDIR") {
							var p = FindFirst(c->parameter, '>');
							//var rw = ChReplace(c->parameter, '\\', '/');
							//var tg = rw;
							string ds{ "" }, dt{ "" };
							if (p < 0) 
								ds = c->parameter;
							else {
								ds = ChReplace(Trim(c->parameter.substr(0, p)), '\\', '/'); //c.parameter.Substring(0, to).Trim().Replace("\\", "/");
								dt = ChReplace(Trim(c->parameter.substr(p + 1)), '\\', '/')+"/";
							}
							//if (DirectoryExists(ds)) {
							if (IsDir(ds)) {
								auto p = GetTree(ds);
								if ((!p->size()) && JQL_VerboseNonCriticalErrors) { cout << "\7\x1b[94mWARNING!\t\x1b[0mDirectory " << ds << " was requested for RAWDIR, but turns out to be empty!\n"; }
								for (auto f : *p) {
									string rw{ ds + "/" + f };
									rw = StReplace(rw, "//", "/");
									if (_JT_Dir::Recognize(rw) != "NONE") {
										var ijcr = JCR6_Dir(rw);
										for (var eij : ijcr->_Entries) {
											auto  nname{ dt + "/" + f + "/" + eij.second->Name() };
											while (nname[0] == '/') nname = nname.substr(1);
											eij.second->Name(nname);
											ret->_Entries[Upper(eij.second->Name())] = eij.second;
										}
									} else {
										var e = make_shared<_JT_Entry>(); //= new TJCREntry();
										var ename{ StReplace(fpath + dt,"//","/") };
										e->_ConfigString["__Entry"] = ename;
										if (e->Name().size()) e->_ConfigString["__Entry"] += "/";
										e->Name(e->_ConfigString["__Entry"] + f); //e.Entry = tg;
										//cout << "Raw: " << rw << " to " << e->Name() << "\n"; // debug only
										e->MainFile = rw;
										e->_ConfigString["__Storage"] = "Store";
										e->_ConfigInt["__Offset"] = 0;
										e->_ConfigInt["__Size"] = FileSize(rw); //(int)new FileInfo(rw).Length;
										e->_ConfigInt["__CSize"] = e->RealSize();
										e->_ConfigInt["__Offset"] = 0;
										e->_ConfigString["__Notes"] = notes;
										e->_ConfigString["__Author"] = author;
										ret->_Entries[Upper(e->Name())] = e;
									}
								}
														
							} else {
								if (!optional) throw runtime_error("Required raw Directory '" + ds + "' not found");
								if (JQL_VerboseNonCriticalErrors) {
									cout << "\7\x1b[94mWARNING!\t\x1b[0mDirectory " << ds << " was requested for RAWDIR, but doesn't seem to exist at all\t(Called from "<<CurrentDir()<<")\n";
								}
							}
						}
						ccase("PLATFORM") {} // Prevent crap
						ccase2("TEXT", "TXT") {
							var tg = ChReplace(Trim(c->parameter), '\\', '/'); //c.parameter.Trim().Replace("\\", "/");
							if (tg.size() > 1 && tg[1] == ':') tg = tg.substr(2);
							while (tg[1] == '/') tg = tg.substr(1);
							if (tg == "") throw runtime_error("TEXT no target");
							var e = make_shared<_JT_Entry>(); //= new TJCREntry();
							char buf[6]; buf[5] = 0; //= new byte[5];
							e->_ConfigString["__Entry"] = fpath + tg;
							e->MainFile = file;
							e->_ConfigString["__Storage"] = "Store";
							e->_ConfigInt["__Offset"] = (int)BT->Position();
							e->_ConfigString["__Notes"] = notes;
							e->_ConfigString["__Author"] = author;
							do {
								if (BT->EndOfFile()) throw runtime_error("Unexpected end of file (TXT Block not ended)");
								for (int i = 0; i < 4; i++) buf[i] = buf[i + 1];
								buf[4] = BT->ReadChar();
								//Console.WriteLine(Encoding.UTF8.GetString(buf, 0, buf.Length));
							} while (strcmp(buf, "@END@") != 0); //while (Encoding.UTF8.GetString(buf, 0, buf.Length) != "@END@");
							RL(BT);
							e->_ConfigInt["__Size"] = (int)(BT->Position() - 7) - e->Offset();
							e->_ConfigInt["__CSize"] = e->RealSize();
							ret->_Entries[Upper(e->Name())] = e;
							//break;
						}
						ccase2("COMMENT", "CMT") {
							if (c->parameter == "") throw runtime_error("Comment without a name");
							var cmt{ string() };//= new StringBuilder("");
							var l{ string() };
							do {
								if (BT->EndOfFile()) throw runtime_error("Unexpected end of file (COMMENT block not ended)");
								l = RL(BT, false);
								if (Trim(l) != "@END@")
									cmt += l + "\n"; // .Append($"{l}\n");
							} while (Trim(l) != "@END@");
							ret->Comments[c->parameter] = cmt; //.ToString();
							//break;
						}
						ccase("IMPORT")
							ret->Patch(c->parameter, fpath);
						//break;
						ccase("FROM") {
							auto P = Upper(c->parameter);
							if (MapFrom.count(P)) {
								From = MapFrom[P];
							} else {
								auto F{ JCR6_Dir(P) };
								if (Last()->Error) throw runtime_error(Last()->ErrorMessage.c_str());
								From = F;
								MapFrom[P] = F;
							}
						}
						ccase("STEAL") {
							if (!From) throw runtime_error("FROM must be defined before you can STEAL");
							var p = FindFirst(c->parameter, '>');
							var rw = ChReplace(c->parameter, '\\', '/');
							var tg = rw;
							if (p >= 0) {
								rw = ChReplace(Trim(c->parameter.substr(0, p)), '\\', '/'); //c.parameter.Substring(0, to).Trim().Replace("\\", "/");
								tg = ChReplace(Trim(c->parameter.substr(p + 1)), '\\', '/'); //c.parameter.Substring(to + 1).Trim().Replace("\\", "/");

							}
							if (tg.size() > 1 && tg[1] == ':') tg = tg.substr(2);
							while (tg[1] == '/') tg = tg.substr(1);
							if (rw == "") throw runtime_error("STEAL no entry");
							if (tg == "") throw runtime_error("STEAL no target");
							auto ei{ From->Entry(rw) };
							if (!ei) {
								std::cout << "\x1b[31mERROR:\x1b[0m Entry to steal turns out to be a null pointer(Entry:" << rw << " from target : " << tg << ")\n";
								throw std::runtime_error("Entry to steal turns out to be a null pointer (Entry:" + rw + " from target:" + tg + ")"); 
							}
							auto eor{ new _JT_Entry() }; *eor = *ei;
							auto eo{ shared_ptr<_JT_Entry>(eor) };
							eo->_ConfigString["__Entry"] = fpath + tg;
							ret->_Entries[Upper(eo->Name())] = eo;
						} 
						ccase("END") {
							return ret;
							//default: throw new Exception($"Unknown instruction! {c.commando}");
						} else {
							throw runtime_error(TrSPrintF("Unknown instrunction! %s", c->commando.c_str()));                            
						}
					einde:
						;
					}
				}
				return ret;
			} catch (runtime_error r) { //catch (Exception e) {
				//JCR6.JERROR = $"JQL error: {e.Message}";
				JCR6_Panic(TrSPrintF("JQL error: %s", r.what()));
				//#ifdef DEBUG
									//Console.WriteLine(e.StackTrace);
				//#endif
				return nullptr;
				//                } finally {
				//                    if (BT != null) BT.Close();
			}
		}


#pragma endregion

#pragma region JSQL
		bool JSQL_Recognize(std::string file) {
			if (!FileExists(file)) return false;
			return (Left(Upper(FLoadString(file)), 5) == "JSQL:");
		}

		JT_Dir JSQL_Dir(std::string file, std::string fpath) {
			auto content{ FLoadString(file) };
			auto lfile{ Trim(Right(content,content.size() - 5)) };
			if (DirectoryExists(lfile)) {
				fpath = ChReplace(fpath, '\\', '/');
				if (!Suffixed(fpath, "/")) fpath += "/";
				var ret = new _JT_Dir();
				var d = GetTree(lfile);
				for(var f : *d) {
					var e = make_shared<_JT_Entry>();
					e->MainFile = lfile + "/" + f;
					e->Name(StripDir(fpath+f));
					e->Storage("Store");
					e->RealSize( (int)FileSize(e->MainFile));
					e->CompressedSize(e->RealSize());
					e->Notes("Linked to by: " + file);
					ret->_Entries[Upper(e->Name())] = e;
				}
				return std::shared_ptr<_JT_Dir>(ret);
			}
			if (_JT_Dir::Recognize(lfile) == "NONE") {
				fpath = ChReplace(fpath, '\\', '/');
				if (!Suffixed(fpath, "/")) fpath += "/";
				auto ret{ make_shared<_JT_Dir>() };
				auto e{ make_shared<_JT_Entry>() };
				e->Name(fpath + StripDir(lfile));
				e->MainFile = lfile;
				e->Storage("Store");
				e->RealSize(FileSize(lfile));
				e->CompressedSize(e->RealSize());
				e->Notes("Linked to by: " + file);
				ret->_Entries[Upper(e->Name())] = e;
				return ret;
			}
			return JCR6_Dir(file, fpath);
		}
		
#pragma endregion

#pragma region Ini
		//public JCR_QuickLink() {
		//    JCR6.FileDrivers["JCR6 Quick Link"] = this;
		//}
		void InitJQL() {
			Chat("Init JQL");
			JD_DirDriver JQL;
			JQL.Name = "JQL";
			JQL.Recognize = JQL_Recognize;
			JQL.Dir = JQL_Dir;
			RegisterDirDriver(JQL);

			Chat("Init JSQL");
			JD_DirDriver JSQL;
			JSQL.Name = "JSQL";
			JSQL.Recognize = JSQL_Recognize;
			JSQL.Dir = JSQL_Dir;
			RegisterDirDriver(JSQL);

		}
	}
#pragma endregion
}
