step_x = 32;
            step_y = 32;

            if (RSpoc > FFMIN(codsty->nreslevels, REpoc))
                continue;

            for (reslevelno = RSpoc; reslevelno < FFMIN(codsty->nreslevels, REpoc); reslevelno++) {
                uint8_t reducedresno = codsty->nreslevels - 1 -reslevelno; //  ==> N_L - r
                Jpeg2000ResLevel *rlevel = comp->reslevel + reslevelno;
                step_x = FFMIN(step_x, rlevel->log2_prec_width  + reducedresno);
                step_y = FFMIN(step_y, rlevel->log2_prec_height + reducedresno);
            }