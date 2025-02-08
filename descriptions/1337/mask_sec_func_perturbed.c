static void stereo_processing(PSContext *ps, INTFLOAT (*l)[32][2], INTFLOAT (*r)[32][2], int is34)
{
    int e, b, k;

    INTFLOAT (*H11)[PS_MAX_NUM_ENV+1][PS_MAX_NR_IIDICC] = ps->H11;
    INTFLOAT (*H12)[PS_MAX_NUM_ENV+1][PS_MAX_NR_IIDICC] = ps->H12;
    INTFLOAT (*H21)[PS_MAX_NUM_ENV+1][PS_MAX_NR_IIDICC] = ps->H21;
    INTFLOAT (*H22)[PS_MAX_NUM_ENV+1][PS_MAX_NR_IIDICC] = ps->H22;
    int8_t *opd_hist = ps->opd_hist;
    int8_t *ipd_hist = ps->ipd_hist;
    int8_t iidremappedbuf[PS_MAX_NUM_ENV][PS_MAX_NR_IIDICC];
    int8_t icc_mapped_buf[PS_MAX_NUM_ENV][PS_MAX_NR_IIDICC];
    int8_t ipd_mapped_buf[PS_MAX_NUM_ENV][PS_MAX_NR_IIDICC];
    int8_t opd_mapped_buf[PS_MAX_NUM_ENV][PS_MAX_NR_IIDICC];
    int8_t (*iid_mapped)[PS_MAX_NR_IIDICC] = iidremappedbuf;
    int8_t (*icc_mapped)[PS_MAX_NR_IIDICC] = icc_mapped_buf;
    int8_t (*ipd_mapped)[PS_MAX_NR_IIDICC] = ipd_mapped_buf;
    int8_t (*opd_mapped)[PS_MAX_NR_IIDICC] = opd_mapped_buf;
    const int8_t *k_to_i = is34 ? k_to_i_34 : k_to_i_20;
    TABLE_CONST INTFLOAT (*H_LUT)[8][4] = (PS_BASELINE || ps->icc_mode < 3) ? HA : HB;

    //Remapping
    if (ps->num_env_old) {
        memcpy(H11[0][0], H11[0][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H11[0][0][0]));
        memcpy(H11[1][0], H11[1][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H11[1][0][0]));
        memcpy(H12[0][0], H12[0][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H12[0][0][0]));
        memcpy(H12[1][0], H12[1][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H12[1][0][0]));
        memcpy(H21[0][0], H21[0][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H21[0][0][0]));
        memcpy(H21[1][0], H21[1][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H21[1][0][0]));
        memcpy(H22[0][0], H22[0][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H22[0][0][0]));
        memcpy(H22[1][0], H22[1][ps->num_env_old], PS_MAX_NR_IIDICC*sizeof(H22[1][0][0]));
    }

    if (is34) {
        remap34(&iid_mapped, ps->iid_par, ps->nr_iid_par, ps->num_env, 1);
        remap34(&icc_mapped, ps->icc_par, ps->nr_icc_par, ps->num_env, 1);
        if (ps->enable_ipdopd) {
            remap34(&ipd_mapped, ps->ipd_par, ps->nr_ipdopd_par, ps->num_env, 0);
            remap34(&opd_mapped, ps->opd_par, ps->nr_ipdopd_par, ps->num_env, 0);
        }
        if (!ps->is34bands_old) {
            map_val_20_to_34(H11[0][0]);
            map_val_20_to_34(H11[1][0]);
            map_val_20_to_34(H12[0][0]);
            map_val_20_to_34(H12[1][0]);
            map_val_20_to_34(H21[0][0]);
            map_val_20_to_34(H21[1][0]);
            map_val_20_to_34(H22[0][0]);
            map_val_20_to_34(H22[1][0]);
            ipdopd_reset(ipd_hist, opd_hist);
        }
    } else {
        remap20(&iid_mapped, ps->iid_par, ps->nr_iid_par, ps->num_env, 1);
        remap20(&icc_mapped, ps->icc_par, ps->nr_icc_par, ps->num_env, 1);
        if (ps->enable_ipdopd) {
            remap20(&ipd_mapped, ps->ipd_par, ps->nr_ipdopd_par, ps->num_env, 0);
            remap20(&opd_mapped, ps->opd_par, ps->nr_ipdopd_par, ps->num_env, 0);
        }
        if (ps->is34bands_old) {
            map_val_34_to_20(H11[0][0]);
            map_val_34_to_20(H11[1][0]);
            map_val_34_to_20(H12[0][0]);
            map_val_34_to_20(H12[1][0]);
            map_val_34_to_20(H21[0][0]);
            map_val_34_to_20(H21[1][0]);
            map_val_34_to_20(H22[0][0]);
            map_val_34_to_20(H22[1][0]);
            ipdopd_reset(ipd_hist, opd_hist);
        }
    }

    //Mixing
    for (e = 0; e < ps->num_env; e++) {
        // <MASK>
    }
}