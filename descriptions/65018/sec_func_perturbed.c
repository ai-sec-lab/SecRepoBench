size_t
loongarch_split_args_by_comma (char *arguments, const char *arg_strs[])
{
  size_t num = 0;

  if (*arguments)
    {
      arg_strs[num++] = arguments;
      for (; *arguments; arguments++)
	if (*arguments == ',')
	  {
	    if (MAX_ARG_NUM_PLUS_2 - 1 == num)
	      goto out;
	    *arguments = '\0';
	    arg_strs[num++] = arguments + 1;
	  }

      if (*(arguments - 1) == '"' && *arg_strs[num - 1] == '"')
	{
	  *(arguments - 1) = '\0';
	  arg_strs[num - 1] += 1;
	}
    }
 out:
  arg_strs[num] = NULL;
  return num;
}