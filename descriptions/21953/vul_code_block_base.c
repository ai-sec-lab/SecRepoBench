/* Invoke the filter callback */
            ret = f_ins->p->cb_filter(work_data,      /* msgpack buffer   */
                                      work_size,      /* msgpack size     */
                                      tag, tag_len,   /* input tag        */
                                      &out_buf,       /* new data         */
                                      &out_size,      /* new data size    */
                                      f_ins,          /* filter instance  */
                                      f_ins->context, /* filter priv data */
                                      config);

            /* Override buffer just if it was modified */
            if (ret == FLB_FILTER_MODIFIED) {
                /* all records removed, no data to continue processing */
                if (out_size == 0) {
                    /* reset data content length */
                    flb_input_chunk_write_at(ic, write_at, "", 0);

#ifdef FLB_HAVE_METRICS
                    ic->total_records = pre_records;

                    /* Summarize all records removed */
                    flb_metrics_sum(FLB_METRIC_N_DROPPED,
                                    in_records, f_ins->metrics);
#endif
                    break;
                }
                else {
#ifdef FLB_HAVE_METRICS
                    out_records = flb_mp_count(out_buf, out_size);
                    if (out_records > in_records) {
                        diff = (out_records - in_records);
                        /* Summarize new records */
                        flb_metrics_sum(FLB_METRIC_N_ADDED,
                                        diff, f_ins->metrics);
                    }
                    else if (out_records < in_records) {
                        diff = (in_records - out_records);
                        /* Summarize dropped records */
                        flb_metrics_sum(FLB_METRIC_N_DROPPED,
                                        diff, f_ins->metrics);
                    }

                    /* set number of records in new chunk */
                    in_records = out_records;
                    ic->total_records = pre_records + in_records;
#endif
                }
                ret = flb_input_chunk_write_at(ic, write_at,
                                               out_buf, out_size);
                if (ret == -1) {
                    flb_error("[filter] could not write data to storage. "
                              "Skipping filtering.");
                    flb_free(out_buf);
                    continue;
                }

                /* Point back the 'data' pointer to the new address */
                ret = cio_chunk_get_content(ic->chunk,
                                            (char **) &work_data, &cur_size);
                if (ret != CIO_OK) {
                    flb_error("[filter] error retrieving data chunk");
                }
                else {
                    work_data += (cur_size - out_size);
                    work_size = out_size;
                }
                flb_free(out_buf);
            }