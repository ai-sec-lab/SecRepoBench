{
                matvar->data_size = (int)Mat_SizeOf(matvar->data_type);
                data_type_id = DataType2H5T(matvar->data_type);
            } else if ( MAT_C_STRUCT == matvar->class_type ) {
                /* Empty structure array */
                break;
            } else {
                matvar->data_size = (int)Mat_SizeOfClass(matvar->class_type);
                data_type_id = ClassType2H5T(matvar->class_type);
            }