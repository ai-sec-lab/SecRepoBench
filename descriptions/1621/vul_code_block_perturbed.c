rate_ti = proto_tree_add_float_format(user_tree,
									hf_radiotap_vht_datarate[i],
									tvb, offset, 12, rate,
									"Data Rate: %.1f Mb/s", rate);
							PROTO_ITEM_SET_GENERATED(rate_ti);
							if (ieee80211_vhtvalid[mcs].valid[bandwidth][nss] == FALSE)
								expert_add_info(packetinfo, rate_ti, &ei_radiotap_invalid_data_rate);