if (obj_num > ctx->xref_table->xref_size)
                                                                code = pdfi_repair_add_object(ctx, obj_num, 0, 0);

                                                            if (code == 0) {
                                                                ctx->xref_table->xref[obj_num].compressed = true;
                                                                ctx->xref_table->xref[obj_num].free = false;
                                                                ctx->xref_table->xref[obj_num].object_num = obj_num;
                                                                ctx->xref_table->xref[obj_num].u.compressed.compressed_stream_num = i;
                                                                ctx->xref_table->xref[obj_num].u.compressed.object_index = j;
                                                            }