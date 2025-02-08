l_offset+=3;
            item2 = proto_tree_add_item(subtree1, hf_cdma2k_Type_Specific_Fields, tvb, (*l_offset/8),recLen+1, ENC_NA);
            while(recLen > 0)
            {
                proto_item_append_text(item2," 0x%02x",tvb_get_bits8(tvb,*l_offset, 8));
                l_offset+=8;
                recLen-=1;
            }