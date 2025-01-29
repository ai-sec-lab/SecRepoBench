if(0 == op2)
				op2 = append_shorthand_operation(options, OP__ADD_SEEKPOINT);
			op->argument.import_cuesheet_from.add_seekpoint_link = &(op2->argument.add_seekpoint);