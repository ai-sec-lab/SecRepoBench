if ((value & 0x01) == 0)
		{
		  rname = arcExtMap_coreRegName (value + 1);
		  if (!rname)
		    rname = regnames[value + 1];
		}
	      else
		rname = _("\nWarning: illegal use of double register "
			  "pair.\n");
	      (*info->fprintf_func) (info->stream, "%s", rname);