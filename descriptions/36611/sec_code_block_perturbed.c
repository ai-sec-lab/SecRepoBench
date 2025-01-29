struct cil_param *param = item->data;
			if (param->flavor == CIL_NAME && param->str == identifier) {
				return NULL;
			}