ctx->xref_table->xref[obj_num].compressed = true;
                                                            ctx->xref_table->xref[obj_num].free = false;
                                                            ctx->xref_table->xref[obj_num].object_num = obj_num;
                                                            ctx->xref_table->xref[obj_num].u.compressed.compressed_stream_num = i;
                                                            ctx->xref_table->xref[obj_num].u.compressed.object_index = j;