switch (ar->type) {
		case GF_VVC_NALU_DEC_PARAM:
		case GF_VVC_NALU_OPI:
		case GF_VVC_NALU_VID_PARAM:
		case GF_VVC_NALU_SEQ_PARAM:
		case GF_VVC_NALU_PIC_PARAM:
		case GF_VVC_NALU_SEI_PREFIX:
		case GF_VVC_NALU_SEI_SUFFIX:
			valid = GF_TRUE;
			ar->nalus = gf_list_new();
			gf_list_add(cfg->param_array, ar);
			break;
		default:
			GF_LOG(GF_LOG_WARNING, GF_LOG_CODING, ("[VVC] Invalid NALU type %d in vvcC - ignoring\n", ar->type));
			gf_free(ar);
			break;
		}

		if (!valid || ((ar->type != GF_VVC_NALU_DEC_PARAM) && (ar->type != GF_VVC_NALU_OPI)))
			nalucount = gf_bs_read_int(bs, 16);
		else
			nalucount = 1;
			
		for (sublayerIndex=0; sublayerIndex<nalucount; sublayerIndex++) {
			GF_NALUFFParam *sl;
			u32 size = gf_bs_read_int(bs, 16);
			if ((size>gf_bs_available(bs)) || (size<2)) {
				GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[VVC] Wrong param set size %d\n", size));
				gf_odf_vvc_cfg_del(cfg);
				return NULL;
			}
			if (!valid) {
				gf_bs_skip_bytes(bs, size);
				continue;
			}
			GF_SAFEALLOC(sl, GF_NALUFFParam );
			if (!sl) {
				gf_odf_vvc_cfg_del(cfg);
				GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[VVC] alloc failed while parsing vvc config\n"));
				return NULL;
			}

			sl->size = size;
			sl->data = (char *)gf_malloc(sizeof(char) * sl->size);
			if (!sl->data) {
				gf_free(sl);
				gf_odf_vvc_cfg_del(cfg);
				GF_LOG(GF_LOG_ERROR, GF_LOG_CODING, ("[VVC] alloc failed while parsing vvc config\n"));
				return NULL;
			}
			gf_bs_read_data(bs, sl->data, sl->size);
			gf_list_add(ar->nalus, sl);
		}