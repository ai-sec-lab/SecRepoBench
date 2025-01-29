int operandindex;

  for (operandindex = 0; instruction->operands[operandindex].op_type && operandindex < MAX_OPERANDS; operandindex++)
    ;

  return operandindex;