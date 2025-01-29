switch ( class_type ) {
        case 0:
            matvar->class_type = MAT_C_DOUBLE;
            break;
        case 1:
            matvar->class_type = MAT_C_CHAR;
            break;
        case 2:
            matvar->class_type = MAT_C_SPARSE;
            break;
        default:
            Mat_VarFree(matvar);
            return NULL;
    }

    matvar->rank = 2;
    matvar->dims = (size_t*)calloc(2, sizeof(*matvar->dims));
    if ( NULL == matvar->dims ) {
        Mat_VarFree(matvar);
        return NULL;
    }