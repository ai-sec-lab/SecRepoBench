if ((unsigned int) data < module->file_table_count)
		    {
		      curr_srec->sfile = data;
		      curr_srec->srec = module->file_table[data].srec;
		    }
		  vms_debug2 ((4, "DST_S_C_SRC_SETFILE: %d\n", data));
		  break;