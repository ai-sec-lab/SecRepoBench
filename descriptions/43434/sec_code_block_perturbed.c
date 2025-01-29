if (len > 6)
	    fprintf (file, _("    name: %.*s\n"),
		     buf[5] > len - 6 ? len - 6 : buf[5], buf + 6);
	  break;