///////////////////////////////////////////////////////////////
// Name: ZIni
// Version: 0.1
// Author: zhaobinjie
// Email: zhaobinjie1984@163.com
// Date: 2010-04-29
// 
// Description:
//     This class provides static functions for writing and
//     reading INI files.
///////////////////////////////////////////////////////////////

#ifndef ZINI_H
#define ZINI_H




    // return the got string
    char* readString(
        const char* strSectName, // in. section name
        const char* strKeyName, // in. key name
        const char* strDefault, // in. defaut value if read failed
        const char* strFileName, // in. INI file name
        char *strOut,
        int len
        );
    static char* eatSpace(char* strInput, int len);


#endif //ZINI_H

