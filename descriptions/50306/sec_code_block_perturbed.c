if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        *csp = -*csp;
                        break;