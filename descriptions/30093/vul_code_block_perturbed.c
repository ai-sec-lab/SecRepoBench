{
        length = snprintf(str, sizeof(str) - 1, "%" PRId64, v->val.i64);
        tmp = flb_sds_cat(buf, str, length);
    }
    else if (v->type == FLB_RA_FLOAT) {
        length = snprintf(str, sizeof(str) - 1, "%f", v->val.f64);
        tmp = flb_sds_cat(buf, str, length);
    }
    else if (v->type == FLB_RA_STRING) {
        tmp = flb_sds_cat(buf, v->val.string, flb_sds_len(v->val.string));
    }
    else if (v->type == FLB_RA_NULL) {
        tmp = flb_sds_cat(buf, "null", 4);
    }