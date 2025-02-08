case V850_OPERAND_REG:
		  info->fprintf_func (info->stream, "%s", get_v850_reg_name (value));
		  break;
		case (V850_OPERAND_REG|V850_REG_EVEN):
		  info->fprintf_func (info->stream, "%s", get_v850_reg_name (value * 2));
		  break;
		case V850_OPERAND_EP:
		  info->fprintf_func (info->stream, "ep");
		  break;
		case V850_OPERAND_SRG:
		  info->fprintf_func (info->stream, "%s", get_v850_sreg_name (value));
		  break;
		case V850E_OPERAND_REG_LIST:
		  {
		    static int list12_regs[32]   = { 30, 0, 0, 0, 0, 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
						     0,  0, 0, 0, 0, 31, 29, 28, 23, 22, 21, 20, 27, 26, 25, 24 };
		    int *regs;
		    int i;
		    unsigned long int mask = 0;
		    int pc = 0;

		    switch (operand->shift)
		      {
		      case 0xffe00001: regs = list12_regs; break;
		      default:
			/* xgettext:c-format */
			opcodes_error_handler (_("unknown operand shift: %x"),
					       operand->shift);
			abort ();
		      }

		    for (i = 0; i < 32; i++)
		      {
			if (value & (1 << i))
			  {
			    switch (regs[ i ])
			      {
			      default:
				mask |= (1 << regs[ i ]);
				break;
			      case 0:
				/* xgettext:c-format */
				opcodes_error_handler (_("unknown reg: %d"), i);
				abort ();
				break;
			      case -1:
				pc = 1;
				break;
			      }
			  }
		      }

		    info->fprintf_func (info->stream, "{");

		    if (mask || pc)
		      {
			if (mask)
			  {
			    unsigned int bit;
			    int shown_one = 0;

			    for (bit = 0; bit < 32; bit++)
			      if (mask & (1 << bit))
				{
				  unsigned long int first = bit;
				  unsigned long int last;

				  if (shown_one)
				    info->fprintf_func (info->stream, ", ");
				  else
				    shown_one = 1;

				  info->fprintf_func (info->stream, "%s", get_v850_reg_name (first));

				  for (bit++; bit < 32; bit++)
				    if ((mask & (1 << bit)) == 0)
				      break;

				  last = bit;

				  if (last > first + 1)
				    {
				      info->fprintf_func (info->stream, " - %s", get_v850_reg_name (last - 1));
				    }
				}
			  }

			if (pc)
			  info->fprintf_func (info->stream, "%sPC", mask ? ", " : "");
		      }

		    info->fprintf_func (info->stream, "}");
		  }
		  break;

		case V850_OPERAND_CC:
		  info->fprintf_func (info->stream, "%s", get_v850_cc_name (value));
		  break;

		case V850_OPERAND_FLOAT_CC:
		  info->fprintf_func (info->stream, "%s", get_v850_float_cc_name (value));
		  break;

		case V850_OPERAND_CACHEOP:
		  {
		    int idx;

		    for (idx = 0; v850_cacheop_codes[idx] != -1; idx++)
		      {
			if (value == v850_cacheop_codes[idx])
			  {
			    info->fprintf_func (info->stream, "%s",
						get_v850_cacheop_name (idx));
			    goto MATCH_CACHEOP_CODE;
			  }
		      }
		    info->fprintf_func (info->stream, "%d", (int) value);
		  }
		MATCH_CACHEOP_CODE:
		  break;

		case V850_OPERAND_PREFOP:
		  {
		    int idx;

		    for (idx = 0; v850_prefop_codes[idx] != -1; idx++)
		      {
			if (value == v850_prefop_codes[idx])
			  {
			    info->fprintf_func (info->stream, "%s",
						get_v850_prefop_name (idx));
			    goto MATCH_PREFOP_CODE;
			  }
		      }
		    info->fprintf_func (info->stream, "%d", (int) value);
		  }
		MATCH_PREFOP_CODE:
		  break;

		case V850_OPERAND_VREG:
		  info->fprintf_func (info->stream, "%s", get_v850_vreg_name (value));
		  break;