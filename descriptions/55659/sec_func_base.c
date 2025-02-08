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
    parameters;

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
    parameters=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
    if (parameters > 0)
      {
        /*
          Do not expand command option parameters.
        */
        for (j=0; j < parameters; j++)
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
    if (IsPathAccessible(option) != MagickFalse)
      continue;
    if (*option != '@')
      {
        /*
          Generate file list from wildcard filename (e.g. *.jpg).
        */
        GetPathComponent(option,HeadPath,path);
        GetPathComponent(option,SubimagePath,subimage);
        ExpandFilename(path);
        if (*home_directory == '\0')
          getcwd_utf8(home_directory,MagickPathExtent-1);
        filelist=ListFiles(*path == '\0' ? home_directory : path,filename,
          &number_files);
      }
    else
      {
        char
          *files;

        ExceptionInfo
          *exception;

        int
          length;

        /*
          Generate file list from file list (e.g. @filelist.txt).
        */
        exception=AcquireExceptionInfo();
        files=FileToString(option,~0UL,exception);
        exception=DestroyExceptionInfo(exception);
        if (files == (char *) NULL)
          continue;
        filelist=StringToArgv(files,&length);
        if (filelist == (char **) NULL)
          continue;
        files=DestroyString(files);
        filelist[0]=DestroyString(filelist[0]);
        for (j=0; j < (ssize_t) (length-1); j++)
          filelist[j]=filelist[j+1];
        number_files=(size_t) length-1;
      }
    if (filelist == (char **) NULL)
      continue;
    for (j=0; j < (ssize_t) number_files; j++)
      if (IsPathDirectory(filelist[j]) <= 0)
        break;
    if (j == (ssize_t) number_files)
      {
        for (j=0; j < (ssize_t) number_files; j++)
          filelist[j]=DestroyString(filelist[j]);
        filelist=(char **) RelinquishMagickMemory(filelist);
        continue;
      }
    /*
      Transfer file list to argument vector.
    */
    vector=(char **) ResizeQuantumMemory(vector,(size_t) *number_arguments+
      count+number_files+1,sizeof(*vector));
    if (vector == (char **) NULL)
      {
        for (j=0; j < (ssize_t) number_files; j++)
          filelist[j]=DestroyString(filelist[j]);
        filelist=(char **) RelinquishMagickMemory(filelist);
        return(MagickFalse);
      }
    for (j=0; j < (ssize_t) number_files; j++)
    {
      option=filelist[j];
      parameters=ParseCommandOption(MagickCommandOptions,MagickFalse,option);
      if (parameters > 0)
        {
          ssize_t
            k;

          /*
            Do not expand command option parameters.
          */
          vector[count++]=ConstantString(option);
          for (k=0; k < parameters; k++)
          {
            j++;
            if (j == (ssize_t) number_files)
              break;
            option=filelist[j];
            vector[count++]=ConstantString(option);
          }
          continue;
        }
      (void) CopyMagickString(filename,path,MagickPathExtent);
      if (*path != '\0')
        (void) ConcatenateMagickString(filename,DirectorySeparator,
          MagickPathExtent);
      if (filelist[j] != (char *) NULL)
        (void) ConcatenateMagickString(filename,filelist[j],MagickPathExtent);
      filelist[j]=DestroyString(filelist[j]);
      if (strlen(filename) >= (MagickPathExtent-1))
        ThrowFatalException(OptionFatalError,"FilenameTruncated");
      if (IsPathDirectory(filename) <= 0)
        {
          char
            file_path[MagickPathExtent];

          *file_path='\0';
          if (*magick != '\0')
            {
              (void) ConcatenateMagickString(file_path,magick,
                MagickPathExtent);
              (void) ConcatenateMagickString(file_path,":",MagickPathExtent);
            }
          (void) ConcatenateMagickString(file_path,filename,MagickPathExtent);
          if (*subimage != '\0')
            {
              (void) ConcatenateMagickString(file_path,"[",MagickPathExtent);
              (void) ConcatenateMagickString(file_path,subimage,
                MagickPathExtent);
              (void) ConcatenateMagickString(file_path,"]",MagickPathExtent);
            }
          if (strlen(file_path) >= (MagickPathExtent-1))
            ThrowFatalException(OptionFatalError,"FilenameTruncated");
          if (destroy != MagickFalse)
            {
              count--;
              vector[count]=DestroyString(vector[count]);
              destroy=MagickFalse;
            }
          vector[count++]=ConstantString(file_path);
        }
    }
    filelist=(char **) RelinquishMagickMemory(filelist);
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