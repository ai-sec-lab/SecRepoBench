static size_t
ReadNextStructField( mat_t *mat, matvar_t *matvar )
{
    mat_uint32_t fieldname_size;
    int err;
    size_t bytesread = 0, nfields, fieldindex;
    matvar_t **fields = NULL;
    size_t nelems = 1, nelems_x_nfields;

    err = SafeMulDims(matvar, &nelems);
    if ( err ) {
        Mat_Critical("Integer multiplication overflow");
        return bytesread;
    }
    if ( matvar->compression == MAT_COMPRESSION_ZLIB ) {
#if HAVE_ZLIB
        mat_uint32_t uncomp_buf[16] = {0,};
        mat_uint32_t array_flags, len;

        /* Field name length */
        bytesread += InflateVarTag(mat,matvar,uncomp_buf);
        if ( mat->byteswap ) {
            (void)Mat_uint32Swap(uncomp_buf);
            (void)Mat_uint32Swap(uncomp_buf+1);
        }
        if ( (uncomp_buf[0] & 0x0000ffff) == MAT_T_INT32 && uncomp_buf[1] > 0 ) {
            fieldname_size = uncomp_buf[1];
        } else {
            Mat_Critical("Error getting fieldname size");
            return bytesread;
        }

        /* Field name tag */
        bytesread += InflateVarTag(mat,matvar,uncomp_buf);
        if ( mat->byteswap )
            (void)Mat_uint32Swap(uncomp_buf);
        /* Name of field */
        if ( uncomp_buf[0] == MAT_T_INT8 ) {    /* Name not in tag */
            if ( mat->byteswap )
                len = Mat_uint32Swap(uncomp_buf+1);
            else
                len = uncomp_buf[1];
            nfields = len / fieldname_size;
            if ( nfields*fieldname_size % 8 != 0 )
                fieldindex = 8-(nfields*fieldname_size % 8);
            else
                fieldindex = 0;
            if ( nfields ) {
                char *ptr = (char*)malloc(nfields*fieldname_size+fieldindex);
                if ( NULL != ptr ) {
                    bytesread += InflateVarName(mat,matvar,ptr,nfields*fieldname_size+fieldindex);
                    SetFieldNames(matvar, ptr, nfields, fieldname_size);
                    free(ptr);
                }
            } else {
                matvar->internal->num_fields = 0;
                matvar->internal->fieldnames = NULL;
            }
        } else {
            len = (uncomp_buf[0] & 0xffff0000) >> 16;
            if ( ((uncomp_buf[0] & 0x0000ffff) == MAT_T_INT8) && len > 0 && len <= 4 ) {
                /* Name packed in tag */
                nfields = len / fieldname_size;
                if ( nfields ) {
                    SetFieldNames(matvar, (char*)(uncomp_buf + 1), nfields, fieldname_size);
                } else {
                    matvar->internal->num_fields = 0;
                    matvar->internal->fieldnames = NULL;
                }
            } else {
                nfields = 0;
            }
        }

        matvar->data_size = sizeof(matvar_t *);
        err = SafeMul(&nelems_x_nfields, nelems, nfields);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            return bytesread;
        }
        err = SafeMul(&matvar->nbytes, nelems_x_nfields, matvar->data_size);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            return bytesread;
        }
        if ( !matvar->nbytes )
            return bytesread;

        matvar->data = calloc(nelems_x_nfields, matvar->data_size);
        if ( NULL == matvar->data ) {
            Mat_Critical("Couldn't allocate memory for the data");
            return bytesread;
        }

        fields = (matvar_t**)matvar->data;
        for ( fieldindex = 0; fieldindex < nelems; fieldindex++ ) {
            size_t k;
            for ( k = 0; k < nfields; k++ ) {
                fields[fieldindex*nfields+k] = Mat_VarCalloc();
            }
        }
        if ( NULL != matvar->internal->fieldnames ) {
            for ( fieldindex = 0; fieldindex < nelems; fieldindex++ ) {
                size_t k;
                for ( k = 0; k < nfields; k++ ) {
                    if ( NULL != matvar->internal->fieldnames[k] ) {
                        fields[fieldindex*nfields+k]->name = strdup(matvar->internal->fieldnames[k]);
                    }
                }
            }
        }

        for ( fieldindex = 0; fieldindex < nelems_x_nfields; fieldindex++ ) {
            mat_uint32_t nBytes;
            /* Read variable tag for struct field */
            bytesread += InflateVarTag(mat,matvar,uncomp_buf);
            if ( mat->byteswap ) {
                (void)Mat_uint32Swap(uncomp_buf);
                (void)Mat_uint32Swap(uncomp_buf+1);
            }
            nBytes = uncomp_buf[1];
            if ( uncomp_buf[0] != MAT_T_MATRIX ) {
                Mat_VarFree(fields[fieldindex]);
                fields[fieldindex] = NULL;
                Mat_Critical("fields[%" SIZE_T_FMTSTR "], Uncompressed type not MAT_T_MATRIX", fieldindex);
                continue;
            } else if ( 0 == nBytes ) {
                /* Empty field: Memory optimization */
                free(fields[fieldindex]->internal);
                fields[fieldindex]->internal = NULL;
                continue;
            }
            fields[fieldindex]->compression = MAT_COMPRESSION_ZLIB;
            bytesread += InflateArrayFlags(mat,matvar,uncomp_buf);
            nBytes -= 16;
            if ( mat->byteswap ) {
                (void)Mat_uint32Swap(uncomp_buf);
                (void)Mat_uint32Swap(uncomp_buf+1);
                (void)Mat_uint32Swap(uncomp_buf+2);
                (void)Mat_uint32Swap(uncomp_buf+3);
            }
            /* Array flags */
            if ( uncomp_buf[0] == MAT_T_UINT32 ) {
               array_flags = uncomp_buf[2];
               fields[fieldindex]->class_type = CLASS_FROM_ARRAY_FLAGS(array_flags);
               fields[fieldindex]->isComplex  = (array_flags & MAT_F_COMPLEX);
               fields[fieldindex]->isGlobal   = (array_flags & MAT_F_GLOBAL);
               fields[fieldindex]->isLogical  = (array_flags & MAT_F_LOGICAL);
               if ( fields[fieldindex]->class_type == MAT_C_SPARSE ) {
                   /* Need to find a more appropriate place to store nzmax */
                   fields[fieldindex]->nbytes = uncomp_buf[3];
               }
            } else {
                Mat_Critical("Expected MAT_T_UINT32 for array tags, got %d",
                    uncomp_buf[0]);
                bytesread+=InflateSkip(mat,matvar->internal->z,nBytes);
            }
            if ( fields[fieldindex]->class_type != MAT_C_OPAQUE ) {
                mat_uint32_t* dims = NULL;
                int do_clean = 0;
                bytesread += InflateRankDims(mat,matvar,uncomp_buf,sizeof(uncomp_buf),&dims);
                if ( NULL == dims )
                    dims = uncomp_buf + 2;
                else
                    do_clean = 1;
                nBytes -= 8;
                if ( mat->byteswap ) {
                    (void)Mat_uint32Swap(uncomp_buf);
                    (void)Mat_uint32Swap(uncomp_buf+1);
                }
                /* Rank and dimension */
                if ( uncomp_buf[0] == MAT_T_INT32 ) {
                    int j;
                    size_t size;
                    fields[fieldindex]->rank = uncomp_buf[1];
                    nBytes -= fields[fieldindex]->rank;
                    fields[fieldindex]->rank /= 4;
                    if ( 0 == do_clean && fields[fieldindex]->rank > 13 ) {
                        int rank = fields[fieldindex]->rank;
                        fields[fieldindex]->rank = 0;
                        Mat_Critical("%d is not a valid rank", rank);
                        continue;
                    }
                    err = SafeMul(&size, fields[fieldindex]->rank, sizeof(*fields[fieldindex]->dims));
                    if ( err ) {
                        if ( do_clean )
                            free(dims);
                        Mat_VarFree(fields[fieldindex]);
                        fields[fieldindex] = NULL;
                        Mat_Critical("Integer multiplication overflow");
                        continue;
                    }
                    fields[fieldindex]->dims = (size_t*)malloc(size);
                    if ( mat->byteswap ) {
                        for ( j = 0; j < fields[fieldindex]->rank; j++ )
                            fields[fieldindex]->dims[j] = Mat_uint32Swap(dims+j);
                    } else {
                        for ( j = 0; j < fields[fieldindex]->rank; j++ )
                            fields[fieldindex]->dims[j] = dims[j];
                    }
                    if ( fields[fieldindex]->rank % 2 != 0 )
                        nBytes -= 4;
                }
                if ( do_clean )
                    free(dims);
                /* Variable name tag */
                bytesread += InflateVarTag(mat,matvar,uncomp_buf);
                nBytes -= 8;
                fields[fieldindex]->internal->z = (z_streamp)calloc(1,sizeof(z_stream));
                if ( fields[fieldindex]->internal->z != NULL ) {
                    err = inflateCopy(fields[fieldindex]->internal->z,matvar->internal->z);
                    if ( err == Z_OK ) {
                        fields[fieldindex]->internal->datapos = ftell((FILE*)mat->fp);
                        if ( fields[fieldindex]->internal->datapos != -1L ) {
                            fields[fieldindex]->internal->datapos -= matvar->internal->z->avail_in;
                            if ( fields[fieldindex]->class_type == MAT_C_STRUCT )
                                bytesread+=ReadNextStructField(mat,fields[fieldindex]);
                            else if ( fields[fieldindex]->class_type == MAT_C_CELL )
                                bytesread+=ReadNextCell(mat,fields[fieldindex]);
                            else if ( nBytes <= (1 << MAX_WBITS) ) {
                                /* Memory optimization: Read data if less in size
                                   than the zlib inflate state (approximately) */
                                err = Mat_VarRead5(mat,fields[fieldindex]);
                                fields[fieldindex]->internal->data = fields[fieldindex]->data;
                                fields[fieldindex]->data = NULL;
                            }
                            (void)fseek((FILE*)mat->fp,fields[fieldindex]->internal->datapos,SEEK_SET);
                        } else {
                            Mat_Critical("Couldn't determine file position");
                        }
                        if ( fields[fieldindex]->internal->data != NULL ||
                             fields[fieldindex]->class_type == MAT_C_STRUCT ||
                             fields[fieldindex]->class_type == MAT_C_CELL ) {
                            /* Memory optimization: Free inflate state */
                            inflateEnd(fields[fieldindex]->internal->z);
                            free(fields[fieldindex]->internal->z);
                            fields[fieldindex]->internal->z = NULL;
                        }
                    } else {
                        Mat_Critical("inflateCopy returned error %s",zError(err));
                    }
                } else {
                    Mat_Critical("Couldn't allocate memory");
                }
            }
            bytesread+=InflateSkip(mat,matvar->internal->z,nBytes);
        }
#else
        Mat_Critical("Not compiled with zlib support");
#endif
    } else {
        mat_uint32_t buf[6] = {0,};
        mat_uint32_t array_flags, len;

        bytesread+=fread(buf,4,2,(FILE*)mat->fp);
        if ( mat->byteswap ) {
            (void)Mat_uint32Swap(buf);
            (void)Mat_uint32Swap(buf+1);
        }
        if ( (buf[0] & 0x0000ffff) == MAT_T_INT32 && buf[1] > 0 ) {
            fieldname_size = buf[1];
        } else {
            Mat_Critical("Error getting fieldname size");
            return bytesread;
        }

        /* Field name tag */
        bytesread+=fread(buf,4,2,(FILE*)mat->fp);
        if ( mat->byteswap )
            (void)Mat_uint32Swap(buf);
        /* Name of field */
        if ( buf[0] == MAT_T_INT8 ) {    /* Name not in tag */
            if ( mat->byteswap )
                len = Mat_uint32Swap(buf+1);
            else
                len = buf[1];
            nfields = len / fieldname_size;
            if ( nfields ) {
                char *ptr = (char*)malloc(nfields*fieldname_size);
                if ( NULL != ptr ) {
                    size_t readresult = fread(ptr, 1, nfields*fieldname_size, (FILE*)mat->fp);
                    bytesread += readresult;
                    if ( nfields*fieldname_size == readresult )
                        SetFieldNames(matvar, ptr, nfields, fieldname_size);
                    else
                        matvar->internal->fieldnames = NULL;
                    free(ptr);
                }
                if ( (nfields*fieldname_size) % 8 ) {
                    (void)fseek((FILE*)mat->fp,8-((nfields*fieldname_size) % 8),SEEK_CUR);
                    bytesread+=8-((nfields*fieldname_size) % 8);
                }
            } else {
                matvar->internal->num_fields = 0;
                matvar->internal->fieldnames = NULL;
            }
       } else {
            len = (buf[0] & 0xffff0000) >> 16;
            if ( ((buf[0] & 0x0000ffff) == MAT_T_INT8) && len > 0 && len <= 4 ) {
                /* Name packed in tag */
                nfields = len / fieldname_size;
                if ( nfields ) {
                    SetFieldNames(matvar, (char*)(buf + 1), nfields, fieldname_size);
                } else {
                    matvar->internal->num_fields = 0;
                    matvar->internal->fieldnames = NULL;
                }
            } else {
                nfields = 0;
            }
        }

        matvar->data_size = sizeof(matvar_t *);
        err = SafeMul(&nelems_x_nfields, nelems, nfields);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            return bytesread;
        }
        err = SafeMul(&matvar->nbytes, nelems_x_nfields, matvar->data_size);
        if ( err ) {
            Mat_Critical("Integer multiplication overflow");
            return bytesread;
        }
        if ( !matvar->nbytes )
            return bytesread;

        matvar->data = malloc(matvar->nbytes);
        if ( NULL == matvar->data )
            return bytesread;

        fields = (matvar_t**)matvar->data;
        for ( fieldindex = 0; fieldindex < nelems; fieldindex++ ) {
            size_t k;
            for ( k = 0; k < nfields; k++ ) {
                fields[fieldindex*nfields+k] = Mat_VarCalloc();
            }
        }
        if ( NULL != matvar->internal->fieldnames ) {
            for ( fieldindex = 0; fieldindex < nelems; fieldindex++ ) {
                size_t k;
                for ( k = 0; k < nfields; k++ ) {
                    if ( NULL != matvar->internal->fieldnames[k] ) {
                        fields[fieldindex*nfields+k]->name = strdup(matvar->internal->fieldnames[k]);
                    }
                }
            }
        }

        for ( fieldindex = 0; fieldindex < nelems_x_nfields; fieldindex++ ) {
            mat_uint32_t nBytes;
            /* Read variable tag for struct field */
            bytesread += fread(buf,4,2,(FILE*)mat->fp);
            if ( mat->byteswap ) {
                (void)Mat_uint32Swap(buf);
                (void)Mat_uint32Swap(buf+1);
            }
            nBytes = buf[1];
            if ( buf[0] != MAT_T_MATRIX ) {
                Mat_VarFree(fields[fieldindex]);
                fields[fieldindex] = NULL;
                Mat_Critical("fields[%" SIZE_T_FMTSTR "] not MAT_T_MATRIX, fpos = %ld", fieldindex,
                    ftell((FILE*)mat->fp));
                return bytesread;
            } else if ( 0 == nBytes ) {
                /* Empty field: Memory optimization */
                free(fields[fieldindex]->internal);
                fields[fieldindex]->internal = NULL;
                continue;
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
                fields[fieldindex]->class_type = CLASS_FROM_ARRAY_FLAGS(array_flags);
                fields[fieldindex]->isComplex  = (array_flags & MAT_F_COMPLEX);
                fields[fieldindex]->isGlobal   = (array_flags & MAT_F_GLOBAL);
                fields[fieldindex]->isLogical  = (array_flags & MAT_F_LOGICAL);
                if ( fields[fieldindex]->class_type == MAT_C_SPARSE ) {
                    /* Need to find a more appropriate place to store nzmax */
                    fields[fieldindex]->nbytes = buf[3];
                }
            }
            /* Rank and dimension */
            {
                size_t nbytes = ReadRankDims(mat, fields[fieldindex], (enum matio_types)buf[4], buf[5]);
                bytesread += nbytes;
                nBytes -= nbytes;
            }
            /* Variable name tag */
            bytesread+=fread(buf,1,8,(FILE*)mat->fp);
            nBytes-=8;
            fields[fieldindex]->internal->datapos = ftell((FILE*)mat->fp);
            if ( fields[fieldindex]->internal->datapos != -1L ) {
                if ( fields[fieldindex]->class_type == MAT_C_STRUCT )
                    bytesread+=ReadNextStructField(mat,fields[fieldindex]);
                else if ( fields[fieldindex]->class_type == MAT_C_CELL )
                    bytesread+=ReadNextCell(mat,fields[fieldindex]);
                (void)fseek((FILE*)mat->fp,fields[fieldindex]->internal->datapos+nBytes,SEEK_SET);
            } else {
                Mat_Critical("Couldn't determine file position");
            }
        }
    }

    return bytesread;
}