n = dict[h];
	      else
		{
		  if (bfd_bread (&n, 1, nbfd) != 1)
		    goto error_return;
		  dict[h] = n;
		}