if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        CS_CHECK_PUSH(csp, cstack);
                        csp[1] = *csp;
                        ++csp;
                        break;