int i;

  for (i = 0; i < MAX_OPERANDS && instruction->operands[i].op_type; i++)
    ;

  return i;