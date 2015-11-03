#pragma once 

//#include "solo_prerequisites.h"
#include <string>
#include <sstream>

namespace solo {

	extern const char __hexshow_map__ [256][3] ;
	
	inline const char * char2hex(uint8_t c) {
		return __hexshow_map__[ c ] ;
	}
	
	inline std::string hexstr(const void * data, size_t len) {
		try {
			std::string ret ;
			ret.reserve(len * 2) ;

			for (size_t i = 0 ; i < len  ; ++i) {
				ret.append(__hexshow_map__[ static_cast<const uint8_t>(static_cast<const char*>(data)[i]) ], 2) ;
			}

			return ret ;
		} catch(...) {
			// do_nothing ;
		}

		return std::string("") ;
	}

	inline std::string hexshow(const void * data, size_t iLen) {

		try {
			std::string retstr ;
			const char * sStr = static_cast<const char*>(data) ;

			size_t i = 0 ;
			size_t iCount = 0 ;
			size_t iPos = 0 , jPos = 0 ; 

			char chTmp ;
			char acASCII[256] ; 
			char acHexBuf[256] ; 

			for (iCount = 0; iCount < iLen; iCount++)
			{
				i = iCount % 16 ;

				if ((sStr[iCount] >= ' ' && sStr[iCount] <= '~'))
					chTmp = sStr[iCount] ;
				else
					chTmp = '*' ;

				if (i == 8)
				{
					jPos += sprintf(acHexBuf+jPos, "- ") ;
					iPos += sprintf(acASCII+iPos, " ") ;
				}
				if (i == 0)
				{
					if (iCount != 0)
					{
						retstr += "  " ;
						retstr += acHexBuf ;
						retstr += " |  " ;
						retstr += acASCII ;
						retstr += '\n' ;
					}

					iPos = 0 ;
					jPos = 0 ;
				}

				jPos += sprintf(acHexBuf+jPos, "%s ", char2hex(sStr[iCount]) ) ;
				iPos += sprintf(acASCII+iPos, "%c", chTmp);
			}
			if (jPos != 0)
			{
				memset(acHexBuf + jPos, ' ', 50 - jPos) ;
				acHexBuf[50] = '\0' ;
				retstr += "  " ;
				retstr += acHexBuf ;
				retstr += " |  " ;
				retstr += acASCII ;
				retstr += '\n' ;
			}

			return retstr ;

		} catch (...) {
		}

		return std::string("") ;
	}



} ;

