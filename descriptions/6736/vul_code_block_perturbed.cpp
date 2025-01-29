sTableDef.szDataFile[80] = '\0';

            for(i = (int)strlen(sTableDef.szDataFile)-1;
                isspace((unsigned char)sTableDef.szDataFile[i]);
                i--)
            {
                sTableDef.szDataFile[i] = '\0';
            }

            AVCRawBinClose(hFile);