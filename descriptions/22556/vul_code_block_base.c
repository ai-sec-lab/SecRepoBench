mat_uint32_t buf[6];
        mat_uint32_t nBytes;
        mat_uint32_t array_flags;

        for ( i = 0; i < nelems; i++ ) {
            int cell_bytes_read;
            mat_uint32_t name_len;
            cells[i] = Mat_VarCalloc();
            if ( !cells[i] ) {
                Mat_Critical("Couldn't allocate memory for cell %" SIZE_T_FMTSTR, i);
                continue;
            }

            /* Read variable tag for cell */
            cell_bytes_read = fread(buf,4,2,(FILE*)mat->fp);

            /* Empty cells at the end of a file may cause an EOF */
            if ( !cell_bytes_read )
                continue;
            bytesread += cell_bytes_read;
            if ( mat->byteswap ) {
                (void)Mat_uint32Swap(buf);
                (void)Mat_uint32Swap(buf+1);
            }
            nBytes = buf[1];
            if ( 0 == nBytes ) {
                /* Empty cell: Memory optimization */
                free(cells[i]->internal);
                cells[i]->internal = NULL;
                continue;
            } else if ( buf[0] != MAT_T_MATRIX ) {
                Mat_VarFree(cells[i]);
                cells[i] = NULL;
                Mat_Critical("cells[%" SIZE_T_FMTSTR "] not MAT_T_MATRIX, fpos = %ld", i,
                    ftell((FILE*)mat->fp));
                break;
            }

            /* Read array flags and the dimensions tag */
            bytesread += fread(buf,4,6,(FILE*)mat->fp);
            if ( mat->byteswap ) {
                (void)Mat_uint32Swap(buf);
                (void)Mat_uint32Swap(buf+1);
                (void)Mat_uint32Swap(buf+2);
                (void)Mat_uint32Swap(buf+3);
                (void)Mat_uint32Swap(buf+4);
                (void)Mat_uint32Swap(buf+5);
            }
            nBytes-=24;
            /* Array flags */
            if ( buf[0] == MAT_T_UINT32 ) {
               array_flags = buf[2];
               cells[i]->class_type = CLASS_FROM_ARRAY_FLAGS(array_flags);
               cells[i]->isComplex  = (array_flags & MAT_F_COMPLEX);
               cells[i]->isGlobal   = (array_flags & MAT_F_GLOBAL);
               cells[i]->isLogical  = (array_flags & MAT_F_LOGICAL);
               if ( cells[i]->class_type == MAT_C_SPARSE ) {
                   /* Need to find a more appropriate place to store nzmax */
                   cells[i]->nbytes = buf[3];
               }
            }
            /* Rank and dimension */
            {
                size_t nbytes = ReadRankDims(mat, cells[i], (enum matio_types)buf[4], buf[5]);
                bytesread += nbytes;
                nBytes -= nbytes;
            }
            /* Variable name tag */
            bytesread+=fread(buf,1,8,(FILE*)mat->fp);
            nBytes-=8;
            if ( mat->byteswap ) {
                (void)Mat_uint32Swap(buf);
                (void)Mat_uint32Swap(buf+1);
            }
            name_len = 0;
            if ( buf[1] > 0 ) {
                /* Name of variable */
                if ( buf[0] == MAT_T_INT8 ) {    /* Name not in tag */
                    name_len = buf[1];
                    if ( name_len % 8 > 0 ) {
                        if ( name_len < UINT32_MAX - 8 + (name_len % 8) ) {
                            name_len = name_len + 8 - (name_len % 8);
                        } else {
                            Mat_VarFree(cells[i]);
                            cells[i] = NULL;
                            break;
                        }
                    }
                    nBytes -= name_len;
                    (void)fseek((FILE*)mat->fp,name_len,SEEK_CUR);
                }
            }
            cells[i]->internal->datapos = ftell((FILE*)mat->fp);
            if ( cells[i]->internal->datapos != -1L ) {
                if ( cells[i]->class_type == MAT_C_STRUCT )
                    bytesread+=ReadNextStructField(mat,cells[i]);
                if ( cells[i]->class_type == MAT_C_CELL )
                    bytesread+=ReadNextCell(mat,cells[i]);
                (void)fseek((FILE*)mat->fp,cells[i]->internal->datapos+nBytes,SEEK_SET);
            } else {
                Mat_Critical("Couldn't determine file position");
            }
        }