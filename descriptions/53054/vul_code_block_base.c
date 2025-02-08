case ce2_hflex:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-5], cstack))
                            return_error(gs_error_invalidfont);
                        CS_CHECK_PUSHN(csp, cstack, 6);
                        csp[6] = fixed_half;	/* fd/100 */
                        csp[4] = *csp, csp[5] = 0;	/* dx6, dy6 */
                        csp[2] = csp[-1], csp[3] = -csp[-4];	/* dx5, dy5 */
                        *csp = csp[-2], csp[1] = 0;	/* dx4, dy4 */
                        csp[-2] = csp[-3], csp[-1] = 0;		/* dx3, dy3 */
                        csp[-3] = csp[-4], csp[-4] = csp[-5];	/* dx2, dy2 */
                        csp[-5] = 0;	/* dy1 */
                        csp += 6;
                        goto flex;
                    case ce2_flex:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-12], cstack))
                            return_error(gs_error_invalidfont);
                        *csp /= 100;	/* fd/100 */
                        goto flex;
                    case ce2_hflex1:
                        if (!CS_CHECK_CSTACK_BOUNDS(&csp[-3], cstack))
                            return_error(gs_error_invalidfont);
                        CS_CHECK_PUSHN(csp, cstack, 4);
                        csp[4] = fixed_half;	/* fd/100 */
                        csp[2] = *csp;          /* dx6 */
                        csp[3] = -(csp[-7] + csp[-5] + csp[-1]);	/* dy6 */
                        *csp = csp[-2], csp[1] = csp[-1];	/* dx5, dy5 */
                        csp[-2] = csp[-3], csp[-1] = 0;		/* dx4, dy4 */
                        csp[-3] = 0;	/* dy3 */
                        csp += 4;
                        goto flex;