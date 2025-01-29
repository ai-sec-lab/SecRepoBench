if (cur_aug_p > end_aug_p) {
                _dwarf_error(dbg, error,
                    DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
                return DW_DLV_ERROR;
            }
            *lsda_enc_out = *(unsigned char *) cur_aug_p;
            ++cur_aug_p;
            break;