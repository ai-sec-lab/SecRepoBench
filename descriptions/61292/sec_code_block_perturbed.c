if(0 == op2) {
				op2 = append_shorthand_operation(options, OP__ADD_SEEKPOINT);
				/* Need to re-find op, because the appending might have caused realloc */
				op = find_shorthand_operation(options, OP__IMPORT_CUESHEET_FROM);
			}
			op->argument.import_cuesheet_from.add_seekpoint_link = &(op2->argument.add_seekpoint);