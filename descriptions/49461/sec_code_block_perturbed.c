for (index=0; index < Tables ->nCurves; index++) {

        nEntries = Tables->TheCurves[index]->nEntries;

        for (j=0; j < nEntries; j++) {

            val = Tables->TheCurves[index]->Table16[j];        
            if (!_cmsWriteUInt16Number(io, val)) return FALSE;
        }
    }