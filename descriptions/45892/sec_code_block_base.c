code = pdfi_trans_begin_simple_group(ctx, bbox, stroked_bbox, isolated, false);

        /* Group was not pushed if error */
        if (code >= 0)
            state->GroupPushed = true;