for (i=0; i < Tables ->nCurves; i++) {

        nEntries = Tables->TheCurves[i]->nEntries;

        for (j=0; j < nEntries; j++) {

            val = Tables->TheCurves[i]->Table16[j];        
            if (!_cmsWriteUInt16Number(io, val)) return FALSE;
        }
    }