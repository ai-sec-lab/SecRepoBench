ParseSkinLump_3DGS_MDL7(szCursor, &szCursor,
            pcMat, skinType, iWidth, iHeight);

    // now we need to skip any other skins ...
    for (unsigned int i = 1; i < iNumSkins; ++i) {
        SizeCheck(szCursor + 3 * sizeof(uint32_t));
        skinType = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);
        iWidth = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);
        iHeight = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);

        SkipSkinLump_3DGS_MDL7(szCursor, &szCursor, skinType, iWidth, iHeight);
        SizeCheck(szCursor);
    }