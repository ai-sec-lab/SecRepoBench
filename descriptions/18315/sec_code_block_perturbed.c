case '0':
	    case '1':
	    case '2':
	      oper = opcode.op + (*s - '0');
	      if (do_size)
		{
		  if (oper->type == RX_Operand_Indirect || oper->type == RX_Operand_Zero_Indirect)
		    PR (PS, "%s", get_size_name (oper->size));
		}
	      else
		switch (oper->type)
		  {
		  case RX_Operand_Immediate:
		    if (do_addr)
		      dis->print_address_func (oper->addend, dis);
		    else if (do_hex
			     || oper->addend > 999
			     || oper->addend < -999)
		      PR (PS, "%#x", oper->addend);
		    else
		      PR (PS, "%d", oper->addend);
		    break;
		  case RX_Operand_Register:
		  case RX_Operand_TwoReg:
		    PR (PS, "%s", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Indirect:
		    PR (PS, "%d[%s]", oper->addend, get_register_name (oper->reg));
		    break;
		  case RX_Operand_Zero_Indirect:
		    PR (PS, "[%s]", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Postinc:
		    PR (PS, "[%s+]", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Predec:
		    PR (PS, "[-%s]", get_register_name (oper->reg));
		    break;
		  case RX_Operand_Condition:
		    PR (PS, "%s", get_condition_name (oper->reg));
		    break;
		  case RX_Operand_Flag:
		    PR (PS, "%s", get_flag_name (oper->reg));
		    break;
		  case RX_Operand_DoubleReg:
		    PR (PS, "%s", get_double_register_name (oper->reg));
		    break;
		  case RX_Operand_DoubleRegH:
		    PR (PS, "%s", get_double_register_high_name (oper->reg));
		    break;
		  case RX_Operand_DoubleRegL:
		    PR (PS, "%s", get_double_register_low_name (oper->reg));
		    break;
		  case RX_Operand_DoubleCReg:
		    PR (PS, "%s", get_double_control_register_name (oper->reg));
		    break;
		  case RX_Operand_DoubleCond:
		    PR (PS, "%s", get_double_condition_name (oper->reg));
		    break;
		  default:
		    PR (PS, "[???]");
		    break;
		  }