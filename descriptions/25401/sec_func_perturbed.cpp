void HMPImporter::ReadFirstSkin(unsigned int iNumSkins, const unsigned char *szCursor,
        const unsigned char **szCursorOut) {
    ai_assert(0 != iNumSkins);
    ai_assert(nullptr != szCursor);

    // read the type of the skin ...
    // sometimes we need to skip 12 bytes here, I don't know why ...
    uint32_t skinType = *((uint32_t *)szCursor);
    szCursor += sizeof(uint32_t);
    if (0 == skinType) {
        szCursor += sizeof(uint32_t) * 2;
        skinType = *((uint32_t *)szCursor);
        szCursor += sizeof(uint32_t);
        if (!skinType)
            throw DeadlyImportError("Unable to read HMP7 skin chunk");
    }
    // read width and height
    uint32_t iWidth = *((uint32_t *)szCursor);
    szCursor += sizeof(uint32_t);
    uint32_t iHeight = *((uint32_t *)szCursor);
    szCursor += sizeof(uint32_t);

    // allocate an output material
    aiMaterial *pcMat = new aiMaterial();

    // read the skin, this works exactly as for MDL7
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

    // setup the material ...
    pScene->mNumMaterials = 1;
    pScene->mMaterials = new aiMaterial *[1];
    pScene->mMaterials[0] = pcMat;

    *szCursorOut = szCursor;
}