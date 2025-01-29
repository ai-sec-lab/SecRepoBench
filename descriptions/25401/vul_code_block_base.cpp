ParseSkinLump_3DGS_MDL7(szCursor, &szCursor,
            pcMat, iType, iWidth, iHeight);

    // now we need to skip any other skins ...
    for (unsigned int i = 1; i < iNumSkins; ++i) {
        iType = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);
        iWidth = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);
        iHeight = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);

        SkipSkinLump_3DGS_MDL7(szCursor, &szCursor, iType, iWidth, iHeight);
        SizeCheck(szCursor);
    }