    nEntries = Tables->TheCurves[0]->nEntries;

    for (index=0; index < Tables ->nCurves; index++) {

        for (j=0; j < nEntries; j++) {

            val = Tables->TheCurves[index]->Table16[j];        
            if (!_cmsWriteUInt16Number(io, val)) return FALSE;
        }
    }