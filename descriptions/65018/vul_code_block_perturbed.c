if (*arguments)
    arg_strs[num++] = arguments;
  for (; *arguments; arguments++)
    if (*arguments == ',')
      {
	if (MAX_ARG_NUM_PLUS_2 - 1 == num)
	  break;
	else
	  *arguments = '\0', arg_strs[num++] = arguments + 1;
      }

  if (*(arguments-1) == '"')
    {
      *(arguments-1) = '\0';
      arg_strs[num-1] = arg_strs[num-1] + 1;
    }

  arg_strs[num] = NULL;
  return num;