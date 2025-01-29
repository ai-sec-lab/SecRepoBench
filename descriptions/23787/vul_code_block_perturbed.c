if (cur_df_path.len + card->cache.current_df->path.len > sizeof card->cache.current_df->path.value
						|| cur_df_path.len > sizeof card->cache.current_df->path.value) {
					sc_file_free(file);
					LOG_FUNC_RETURN(ctx, SC_ERROR_UNKNOWN_DATA_RECEIVED);
				}
				memcpy(card->cache.current_df->path.value + cur_df_path.len,
						card->cache.current_df->path.value,
						card->cache.current_df->path.len);
				memcpy(card->cache.current_df->path.value, cur_df_path.value, cur_df_path.len);