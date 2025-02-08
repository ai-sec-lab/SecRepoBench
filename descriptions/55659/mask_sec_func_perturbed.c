MagickExport MagickBooleanType ExpandFilenames(int *number_arguments,
  char ***arguments)
{
  char
    home_directory[MagickPathExtent],
    **vector;

  ssize_t
    i,
    j;

  size_t
    number_files;

  ssize_t
    count,
    parameterscanberenamedtoparametercount;

  /*
    Allocate argument vector.
  */
  assert(number_arguments != (int *) NULL);
  assert(arguments != (char ***) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  vector=(char **) AcquireQuantumMemory((size_t) (*number_arguments+1),
    sizeof(*vector));
  if (vector == (char **) NULL)
    ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
  /*
    Expand any wildcard filenames.
  */
  *home_directory='\0';
  count=0;
  for (i=0; i < (ssize_t) *number_arguments; i++)
  {
    char
      **filelist,
      filename[MagickPathExtent],
      magick[MagickPathExtent],
      *option,
      path[MagickPathExtent],
      subimage[MagickPathExtent];

    MagickBooleanType
      destroy;

    option=(*arguments)[i];
    *magick='\0';
    *path='\0';
    *filename='\0';
    *subimage='\0';
    number_files=0;
    vector[count++]=ConstantString(option);
    destroy=MagickTrue;
    parameterscanberenamedtoparametercount=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
    if (parameterscanberenamedtoparametercount > 0)
      {
        /*
          Do not expand command option parameters.
        */
        for (j=0; j < parameterscanberenamedtoparametercount; j++)
        {
          i++;
          if (i == (ssize_t) *number_arguments)
            break;
          option=(*arguments)[i];
          vector[count++]=ConstantString(option);
        }
        continue;
      }
    if ((*option == '"') || (*option == '\''))
      continue;
    GetPathComponent(option,TailPath,filename);
    GetPathComponent(option,MagickPath,magick);
    if ((LocaleCompare(magick,"CAPTION") == 0) ||
        (LocaleCompare(magick,"LABEL") == 0) ||
        (LocaleCompare(magick,"PANGO") == 0) ||
        (LocaleCompare(magick,"VID") == 0))
      continue;
    if ((IsGlob(filename) == MagickFalse) && (*option != '@'))
      continue;
    // <MASK>
  }
  vector[count]=(char *) NULL;
  if (IsEventLogging() != MagickFalse)
    {
      char
        *command_line;

      command_line=AcquireString(vector[0]);
      for (i=1; i < count; i++)
      {
        (void) ConcatenateString(&command_line," {");
        (void) ConcatenateString(&command_line,vector[i]);
        (void) ConcatenateString(&command_line,"}");
      }
      (void) LogMagickEvent(ConfigureEvent,GetMagickModule(),
        "Command line: %s",command_line);
      command_line=DestroyString(command_line);
    }
  *number_arguments=(int) count;
  *arguments=vector;
  return(MagickTrue);
}