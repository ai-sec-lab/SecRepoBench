case ce1_callothersubr:
                if (CS_CHECK_CSTACK_BOUNDS(csp, cstack)) {
                    switch (fixed2int_var(*csp)) {
                    default:
                        goto out;
                    case 3:
                        if (csp >= &(cstack[1]))
                            csp -= 2;
                        goto top;
                    case 12:
                    case 13:
                    case 14:
                    case 15:
                    case 16:
                    case 17:
                    case 18:
                        cnext;
                    }
                }
                else {
                    return_error(gs_error_invalidfont);
                }