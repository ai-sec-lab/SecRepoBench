rname = arcExtMap_coreRegName (value + 1);
	      if (!rname)
		rname = regnames[value + 1];
	      (*info->fprintf_func) (info->stream, "%s", rname);