static
cmsBool Write16bitTables(cmsContext ContextID, cmsIOHANDLER* io, _cmsStageToneCurvesData* Tables)
{
    cmsUInt32Number j;
    cmsUInt32Number index;
    cmsUInt16Number val;
    cmsUInt32Number nEntries;

    _cmsAssert(Tables != NULL);
   
    for (index=0; index < Tables ->nCurves; index++) {

        nEntries = Tables->TheCurves[index]->nEntries;

        for (j=0; j < nEntries; j++) {

            val = Tables->TheCurves[index]->Table16[j];        
            if (!_cmsWriteUInt16Number(io, val)) return FALSE;
        }
    }
    return TRUE;

    cmsUNUSED_PARAMETER(ContextID);
}