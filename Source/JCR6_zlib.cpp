// Lic:
// JCR6/Source/JCR6_zlib.cpp
// Slyvina - JCR6 - Driver for zlib
// version: 22.12.13
// Copyright (C) 2019, 2022 Jeroen P. Broks
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

#include <string>
#include <JCR6_zlib.hpp>


#define JCR6ZLIBWARN
#ifdef JCR6ZLIBWARN
#include <iostream>
#endif

static int JZL_Compress(char* Uncompressed, char* Compressed, int size_uncompressed, std::string _M, std::string _E) {
    // code comes later
    uLongf size_compressed = (size_uncompressed * 4) / 3;
    int err = compress((Bytef*)Compressed, &size_compressed, (Bytef*)Uncompressed, (uLong)size_uncompressed);
    if (err != Z_OK) {
        std::string e{ "Error by zlib during compression! (" }; e += std::to_string(err); e + ") ";
        switch (err) {
        case Z_ERRNO: e += "ERRNO (???)"; break;
        case Z_STREAM_ERROR: e += "Stream Error"; break;
        case Z_DATA_ERROR: e += "Data error"; break;
        case Z_MEM_ERROR: e += "Memory error"; break;
        case Z_BUF_ERROR: e += "Buffer error"; break;
        case Z_VERSION_ERROR: e += "Version error"; break;
        default: e += "Unknown error"; break;
        }

#ifdef JCR6ZLIBWARN
        std::cout << "DEBUG: size_compressed = " << size_compressed << "; size_uncompressed = " << size_uncompressed << "\n";
        std::cout << "ERROR!\t" << e << "\n";
#endif
        //jcr6::JamError(e);
        Slyvina::JCR6::JCR6_Panic(e, _M, _E);
        return -1;
    }
    return (int)size_compressed; // Force store as long as this is not fully set up!
}

static bool JZL_Expand(char* Compressed, char* UnCompressed, int size_compressed, int size_uncompressed, std::string _M, std::string _E) {
    uLongf szc = size_uncompressed;
    int err = uncompress((Bytef*)UnCompressed, &szc, (Bytef*)Compressed, size_compressed);
    if (err != Z_OK) {
        std::string e{ "Error by zlib during expanding! (" }; e += std::to_string(err); e + ") ";
        switch (err) {
        case Z_ERRNO: e += "ERRNO (???)"; break;
        case Z_STREAM_ERROR: e += "Stream Error"; break;
        case Z_DATA_ERROR: e += "Data error"; break;
        case Z_MEM_ERROR: e += "Memory error"; break;
        case Z_BUF_ERROR: e += "Buffer error"; break;
        case Z_VERSION_ERROR: e += "Version error"; break;
        default: e += "Unknown error"; break;
        }

#ifdef JCR6ZLIBWARN
        std::cout << "DEBUG: size_compressed = " << size_compressed << "; size_uncompressed = " << size_uncompressed << "\n";
        std::cout << "ERROR!\t" << e << "\n";
#endif
        //jcr6::JamError(e);
        Slyvina::JCR6::JCR6_Panic(e, _M, _E);
    }
    return err == Z_OK;
}

namespace Slyvina {
    namespace JCR6 {
        void init_zlib() {
            JC_CompressDriver Driver;
            Driver.Compress = JZL_Compress;
            Driver.Expand = JZL_Expand;
            Driver.Name = "zlib";
            RegisterCompressDriver(Driver);
        }
    }
}