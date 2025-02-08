static int
_dwarf_gnu_aug_encodings(Dwarf_Debug dbg, char *augmentation,
    Dwarf_Small * aug_data, Dwarf_Unsigned aug_data_len,
    Dwarf_Half address_size,
    unsigned char *pers_hand_enc_out,
    unsigned char *lsda_enc_out,
    unsigned char *fde_begin_enc_out,
    Dwarf_Addr * gnu_pers_addr_out,
    Dwarf_Error * error)
{
    char *nc = 0;
    Dwarf_Small *cur_aug_p = aug_data;
    Dwarf_Small *end_aug_p = aug_data + aug_data_len;

    for (nc = augmentation; *nc; ++nc) {
        char character = *nc;

        switch (character) {
        case 'z':
            /* Means that the augmentation data is present. */
            continue;

        case 'S':
            /*  Indicates this is a signal stack frame.
                Debuggers have to do
                special handling.  We don't need to do more than
                print this flag at the right time, though
                (see dwarfdump where it prints the augmentation
                string).
                A signal stack frame (in some OS's) can only be
                unwound (backtraced) by knowing it is a signal
                stack frame (perhaps by noticing the name of the
                function for the stack frame if the name can be
                found somehow) and figuring
                out (or knowing) how the kernel and libc
                pushed a structure
                onto the stack and loading registers from
                that structure.
                Totally different from normal stack unwinding.
                This flag gives an unwinder a big leg up by
                decoupling the 'hint: this is a stack frame'
                from knowledge like
                the function name (the name might be
                unavailable at unwind time).
            */
            break;

        case 'L':
            // <MASK>
        case 'R':
            /*  Followed by a one byte argument giving the
                pointer encoding for the address
                pointers in the fde. */
            if (cur_aug_p >= end_aug_p) {
                _dwarf_error(dbg, error,
                    DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
                return DW_DLV_ERROR;
            }
            *fde_begin_enc_out = *(unsigned char *) cur_aug_p;
            ++cur_aug_p;
            break;
        case 'P':{
            int res = DW_DLV_ERROR;
            Dwarf_Small *updated_aug_p = 0;
            unsigned char encoding = 0;

            if (cur_aug_p >= end_aug_p) {
                _dwarf_error(dbg, error,
                    DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
                return DW_DLV_ERROR;
            }
            encoding = *(unsigned char *) cur_aug_p;
            *pers_hand_enc_out = encoding;
            ++cur_aug_p;
            if (cur_aug_p > end_aug_p) {
                _dwarf_error(dbg, error,
                    DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
                return DW_DLV_ERROR;
            }
            /*  DW_EH_PE_pcrel makes no sense here, so we turn it
                off via a section pointer of NULL. */
            res = _dwarf_read_encoded_ptr(dbg,
                (Dwarf_Small *) NULL,
                cur_aug_p,
                encoding,
                end_aug_p,
                address_size,
                gnu_pers_addr_out,
                &updated_aug_p,
                error);
            if (res != DW_DLV_OK) {
                return res;
            }
            cur_aug_p = updated_aug_p;
            if (cur_aug_p > end_aug_p) {
                _dwarf_error(dbg, error,
                    DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
                return DW_DLV_ERROR;
            }
            }
            break;
        default:
            _dwarf_error(dbg, error,
                DW_DLE_FRAME_AUGMENTATION_UNKNOWN);
            return DW_DLV_ERROR;

        }
    }
    return DW_DLV_OK;
}