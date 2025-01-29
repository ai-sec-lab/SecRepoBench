case 'n':
		    if (value == 15)
		      func (stream, "%s", "APSR_nzcv");
		    else
		      func (stream, "%s", arm_regnames[value]);
		    break;

		  case 'T':
		    func (stream, "%s", arm_regnames[value + 1]);
		    break;

		  case 'd':
		    func (stream, "%ld", value);
		    break;