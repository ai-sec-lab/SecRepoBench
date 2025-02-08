case ce2_not:
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        *csp = (*csp ? 0 : fixed_1);
                        break;
                    case ce2_store:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-3], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            int i, n = fixed2int_var(*csp);
                            int ind = fixed2int_var(csp[-3]);
                            int offs = fixed2int_var(csp[-2]);
                            float *to;
                            const fixed *from = pcis->transient_array + fixed2int_var(csp[-1]);

                            if (!CS_CHECK_TRANSIENT_BOUNDS(from, pcis->transient_array))
                                return_error(gs_error_invalidfont);

                            if (ind < countof(Registry)) {
                                to = Registry[ind].values + offs;
                                for (i = 0; i < n; ++i)
                                    to[i] = fixed2float(from[i]);
                            }
                        }
                        csp -= 4;
                        break;
                    case ce2_abs:
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        if (*csp < 0)
                            *csp = -*csp;
                        break;
                    case ce2_add:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] += *csp;
                        --csp;
                        break;
                    case ce2_sub:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] -= *csp;
                        --csp;
                        break;
                    case ce2_div:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        if (*csp == 0)
                            return_error(gs_error_invalidfont);
                        csp[-1] = float2fixed((double)csp[-1] / *csp);
                        --csp;
                        break;
                    case ce2_load:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-2], cstack))
                            return_error(gs_error_invalidfont);
                        /* The specification says there is no j (starting index */
                        /* in registry array) argument.... */
                        {
                            int i, n = fixed2int_var(*csp);
                            int ind = fixed2int_var(csp[-2]);
                            const float *from;
                            fixed *to = pcis->transient_array + fixed2int_var(csp[-1]);

                            if (!CS_CHECK_TRANSIENT_BOUNDS(to, pcis->transient_array))
                                return_error(gs_error_invalidfont);
                            if (ind < countof(Registry)) {
                                from = Registry[ind].values;
                                for (i = 0; i < n; ++i)
                                    to[i] = float2fixed(from[i]);
                            }
                        }
                        csp -= 3;
                        break;
                    case ce2_neg:
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        *csp = -*csp;
                        break;
                    case ce2_eq:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        csp[-1] = (csp[-1] == *csp ? fixed_1 : 0);
                        --csp;
                        break;
                    case ce2_drop:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        --csp;
                        break;
                    case ce2_put:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            fixed *to = pcis->transient_array + fixed2int_var(*csp);

                            if (!CS_CHECK_TRANSIENT_BOUNDS(to, pcis->transient_array))
                                return_error(gs_error_invalidfont);

                            *to = csp[-1];
                            csp -= 2;
                        }
                        break;
                    case ce2_get:
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        {
                            fixed *from = pcis->transient_array + fixed2int_var(*csp);
                            if (!CS_CHECK_TRANSIENT_BOUNDS(from, pcis->transient_array))
                                return_error(gs_error_invalidfont);

                            *csp = *from;
                        }
                        break;
                    case ce2_ifelse:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-3], cstack))
                            return_error(gs_error_invalidfont);
                        if (csp[-1] > *csp)
                            csp[-3] = csp[-2];
                        csp -= 3;
                        break;
                    case ce2_random:
                        CS_CHECK_PUSH(csp, cstack);
                        ++csp;
                        /****** NYI ******/
                        break;
                    case ce2_mul:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-1], cstack))
                            return_error(gs_error_invalidfont);
                        {
                            double prod = fixed2float(csp[-1]) * *csp;

                            csp[-1] =
                                (prod > max_fixed ? max_fixed :
                                 prod < min_fixed ? min_fixed : (fixed)prod);
                        }
                        --csp;
                        break;
                    case ce2_sqrt:
                        if (!CS_CHECK_CSTACK_BOUNDS(csp, cstack))
                            return_error(gs_error_invalidfont);
                        if (*csp >= 0)
                            *csp = float2fixed(sqrt(fixed2float(*csp)));
                        break;