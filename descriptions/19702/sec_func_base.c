static int
get_number_of_operands (void)
{
  int i;

  for (i = 0; i < MAX_OPERANDS && instruction->operands[i].op_type; i++)
    ;

  return i;
}