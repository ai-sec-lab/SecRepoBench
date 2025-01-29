n = dict[h];
	      else
		{
		  if (! bfd_bread (&n, (bfd_size_type) 1, nbfd))
		    goto error_return;
		  dict[h] = n;
		}