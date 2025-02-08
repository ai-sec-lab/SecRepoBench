static void EXIFPrintData(char* pszData, GUInt16 type,
                   GUInt32 count, const unsigned char* byteData)
{
  const char* sep = "";
  char  szTemp[128];
  char* pszDataEnd = pszData;

  pszData[0]='\0';

  switch (type) {

  case TIFF_UNDEFINED:
  case TIFF_BYTE:
    for(;count>0;count--) {
      snprintf(szTemp, sizeof(szTemp), "%s0x%02x", sep, *byteData++);
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;

  case TIFF_SBYTE:
    for(;count>0;count--) {
      snprintf(szTemp, sizeof(szTemp), "%s%d", sep, *reinterpret_cast<const char *>(byteData));
      byteData ++;
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;

  case TIFF_ASCII:
    memcpy( pszData, byteData, count );
    pszData[count] = '\0';
    break;

  case TIFF_SHORT: {
    const GUInt16 *wp = reinterpret_cast<const GUInt16 *>(byteData);
    for(;count>0;count--) {
      snprintf(szTemp, sizeof(szTemp), "%s%u", sep, *wp++);
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_SSHORT: {
    const GInt16 *wp = reinterpret_cast<const GInt16 *>(byteData);
    for(;count>0;count--) {
      snprintf(szTemp, sizeof(szTemp), "%s%d", sep, *wp++);
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_LONG: {
    const GUInt32 *lp = reinterpret_cast<const GUInt32 *>(byteData);
    for(;count>0;count--) {
      snprintf(szTemp, sizeof(szTemp), "%s%u", sep, *lp);
      lp++;
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_SLONG: {
    const GInt32 *lp = reinterpret_cast<const GInt32 *>(byteData);
    for(;count>0;count--) {
      snprintf(szTemp, sizeof(szTemp), "%s%d", sep, *lp);
      lp++;
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_RATIONAL: {
    const GUInt32 *lp = reinterpret_cast<const GUInt32 *>(byteData);
      //      if(bSwabflag)
      //      TIFFSwabArrayOfLong((GUInt32*) data, 2*count);
    for(;count>0;count--) {
      if( (lp[0]==0) || (lp[1] == 0) ) {
          snprintf(szTemp, sizeof(szTemp), "%s(0)",sep);
      }
      else{
          CPLsnprintf(szTemp, sizeof(szTemp), "%s(%g)", sep,
              static_cast<double>(lp[0])/ static_cast<double>(lp[1]));
      }
      sep = " ";
      lp += 2;
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_SRATIONAL: {
    const GInt32 *lp = reinterpret_cast<const GInt32 *>(byteData);
    for(;count>0;count--) {
      if( (lp[0]==0) || (lp[1] == 0) ) {
          snprintf(szTemp, sizeof(szTemp), "%s(0)",sep);
      }
      else{
        CPLsnprintf(szTemp, sizeof(szTemp), "%s(%g)", sep,
            static_cast<double>(lp[0])/ static_cast<double>(lp[1]));
      }
      sep = " ";
      lp += 2;
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_FLOAT: {
    const float *fp = reinterpret_cast<const float *>(byteData);
    for(;count>0;count--) {
      CPLsnprintf(szTemp, sizeof(szTemp), "%s%g", sep, *fp++);
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }
  case TIFF_DOUBLE: {
    const double *dp = reinterpret_cast<const double *>(byteData);
    for(;count>0;count--) {
      CPLsnprintf(szTemp, sizeof(szTemp), "%s%g", sep, *dp++);
      sep = " ";
      if (strlen(szTemp) + pszDataEnd - pszData >= MAXSTRINGLENGTH)
          break;
      strcat(pszDataEnd,szTemp);
      pszDataEnd += strlen(pszDataEnd);
    }
    break;
  }

  default:
    return;
  }

  if (type != TIFF_ASCII && count != 0)
  {
      CPLError(CE_Warning, CPLE_AppDefined, "EXIF metadata truncated");
  }
}