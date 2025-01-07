
    cmsUInt32Number j;
    cmsUInt32Number i;
    cmsUInt16Number val;
    cmsUInt32Number nEntries;

    _cmsAssert(Tables != NULL);

    nEntries = Tables->TheCurves[0]->nEntries;

    for (i=0; i < Tables ->nCurves; i++) {

        for (j=0; j < nEntries; j++) {

            val = Tables->TheCurves[i]->Table16[j];        
            if (!_cmsWriteUInt16Number(io, val)) return FALSE;
        }
    }
    return TRUE;

    cmsUNUSED_PARAMETER(ContextID);
