size_t
loongarch_split_args_by_comma (char *args, const char *arg_strs[])
{
  size_t num = 0;

  if (*args)
    {
      arg_strs[num++] = args;
      for (; *args; args++)
	if (*args == ',')
	  {
	    if (MAX_ARG_NUM_PLUS_2 - 1 == num)
	      goto out;
	    *args = '\0';
	    arg_strs[num++] = args + 1;
	  }

      if (*(args - 1) == '"' && *arg_strs[num - 1] == '"')
	{
	  *(args - 1) = '\0';
	  arg_strs[num - 1] += 1;
	}
    }
 out:
  arg_strs[num] = NULL;
  return num;
}