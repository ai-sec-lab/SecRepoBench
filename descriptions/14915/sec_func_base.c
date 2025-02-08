static uint64 _TIFFGetStrileOffsetOrByteCountValue(TIFF *tif, uint32 strile,
                                                   TIFFDirEntry* dirent,
                                                   uint64** parray,
                                                   int *pbErr)
{
    TIFFDirectory *td = &tif->tif_dir;
    if( pbErr )
        *pbErr = 0;
    if( (tif->tif_flags&TIFF_DEFERSTRILELOAD) && !(tif->tif_flags&TIFF_CHOPPEDUPARRAYS) )
    {
        if( !(tif->tif_flags&TIFF_LAZYSTRILELOAD) ||
            /* If the values may fit in the toff_long/toff_long8 member */
            /* then use _TIFFFillStriles to simplify _TIFFFetchStrileValue */
            dirent->tdir_count <= 4 )
        {
            if( !_TIFFFillStriles(tif) )
            {
                if( pbErr )
                    *pbErr = 1;
                /* Do not return, as we want this function to always */
                /* return the same value if called several times with */
                /* the same arguments */
            }
        }
        else
        {
             if( !_TIFFFetchStrileValue(tif, strile, dirent, parray) )
             {
                if( pbErr )
                    *pbErr = 1;
                 return 0;
             }
        }
    }
    if( *parray == NULL || strile >= td->td_nstrips )
    {
        if( pbErr )
            *pbErr = 1;
        return 0;
    }
    return (*parray)[strile];
}