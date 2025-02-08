int i;

  for (i = 0; instruction->operands[i].op_type && i < MAX_OPERANDS; i++)
    ;

  return i;