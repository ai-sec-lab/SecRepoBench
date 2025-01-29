if (*args)
    arg_strs[num++] = args;
  for (; *args; args++)
    if (*args == ',')
      {
	if (MAX_ARG_NUM_PLUS_2 - 1 == num)
	  break;
	else
	  *args = '\0', arg_strs[num++] = args + 1;
      }

  if (*(args-1) == '"')
    {
      *(args-1) = '\0';
      arg_strs[num-1] = arg_strs[num-1] + 1;
    }

  arg_strs[num] = NULL;
  return num;