/********************************************/
                    /* Table 7.7.1.1-1 */
                    /********************************************/

                    guint32 bfwcomphdr_iq_width, bfwcomphdr_comp_meth;
                    proto_item *comp_meth_ti = NULL;

                    /* bfwCompHdr (2 subheaders - bfwIqWidth and bfwCompMeth)*/
                    curroffset = dissect_bfwCompHdr(tvb, extension_tree, curroffset,
                                                &bfwcomphdr_iq_width, &bfwcomphdr_comp_meth, &comp_meth_ti);

                    /* Look up width of samples. */
                    guint8 iq_width = !bfwcomphdr_iq_width ? 16 : bfwcomphdr_iq_width;


                    /* Work out number of bundles, but take care not to divide by zero. */
                    if (numBundPrb == 0) {
                        break;
                    }

                    /* Work out bundles! */
                    ext11_work_out_bundles(startPrbc, numPrbc, numBundPrb, &ext11_settings);
                    num_bundles = ext11_settings.num_bundles;

                    /* Add (complete) bundles */
                    for (guint b=0; b < num_bundles; b++) {

                        curroffset = dissect_bfw_bundle(tvb, extension_tree, pinfo, curroffset,
                                                    comp_meth_ti, bfwcomphdr_comp_meth,
                                                    iq_width,
                                                    b,                                 /* bundle number */
                                                    ext11_settings.bundles[b].start,
                                                    ext11_settings.bundles[b].end,
                                                    ext11_settings.bundles[b].is_orphan);
                        if (!curroffset) {
                            break;
                        }
                    }
                    /* Set flag from last bundle entry */
                    orphaned_prbs = ext11_settings.bundles[num_bundles-1].is_orphan;