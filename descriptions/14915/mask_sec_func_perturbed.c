static uint64 _TIFFGetStrileOffsetOrByteCountValue(TIFF *tiff, uint32 strile,
                                                   TIFFDirEntry* dirent,
                                                   uint64** parray,
                                                   int *pbErr)
{
    TIFFDirectory *td = &tiff->tif_dir;
    if( pbErr )
        *pbErr = 0;
    if( (tiff->tif_flags&TIFF_DEFERSTRILELOAD) && !(tiff->tif_flags&TIFF_CHOPPEDUPARRAYS) )
    {
        if( !(tiff->tif_flags&TIFF_LAZYSTRILELOAD) ||
            /* If the values may fit in the toff_long/toff_long8 member */
            /* then use _TIFFFillStriles to simplify _TIFFFetchStrileValue */
            dirent->tdir_count <= 4 )
        {
            if( !_TIFFFillStriles(tiff) )
            {
                // <MASK>
                /* the same arguments */
            }
        }
        else
        {
             if( !_TIFFFetchStrileValue(tiff, strile, dirent, parray) )
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