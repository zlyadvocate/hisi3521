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

#include "zini.h"
#include <stdio.h>
#include <stdlib.h>



char* readString(const char* strSectName,
                             const char* strKeyName,
                             const char* strDefault,
                             const char* strFileName,
                             char *strOut,
                             int len)
{
    char *result = strDefault;
    // open INI file

    FILE* fsIni = fopen(strFileName, "r");

    // file opened successfully
    if (fsIni)
    {
        // search section
        char strLine[256] = {0};
        char strSection[256] = {0};

        strcat(strSection, "[");
        strcat(strSection, strSectName);
        strcat(strSection, "]");

        while (fgets(strLine, sizeof(strLine),fsIni))
        {

            // section found
            eatSpace(strLine, sizeof(strLine));

            if(strcmp(strSection, strLine) == 0)
            {
                // search key
                while (fgets(strLine, sizeof(strLine),fsIni))
                {

                    eatSpace(strLine, sizeof(strLine));

                    // end of this section
                    if (strlen(strLine) > 0 && '[' == strLine[0])
                    {
                        return result;
                    }

                    char tempKeyName[64] = {0};
                    strncpy(tempKeyName, strLine, strlen(strKeyName));
                    if (strlen(strLine) > strlen(strKeyName) + 1 &&
                        0 == strcmp(tempKeyName, strKeyName) &&
                        '=' == strLine[strlen(strKeyName)])
                    {
                        strcpy(strOut, strLine + strlen(strKeyName) + 1);

                        return strOut;
                    }
                }
            }
        }
    }

    // read failed
    return strDefault;
}





char* eatSpace(char* strInOutput, int len)
{
    int i;
    char* strTmptput = (char*)malloc(len);
    memset(strTmptput, 0, len);
    strncpy(strTmptput, strInOutput, strlen(strInOutput));
    memset(strInOutput, 0, len);

    for (i = 0 ; i < len; i++)
    {
        if (!isspace(strTmptput[i]))
        {
            strInOutput[strlen(strInOutput)] = strTmptput[i];
        }
    }

    free(strTmptput);
    return strInOutput;
}

