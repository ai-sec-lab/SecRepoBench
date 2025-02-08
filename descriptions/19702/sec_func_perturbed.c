static int
get_number_of_operands (void)
{
  int operandindex;

  for (operandindex = 0; operandindex < MAX_OPERANDS && instruction->operands[operandindex].op_type; operandindex++)
    ;

  return operandindex;
}