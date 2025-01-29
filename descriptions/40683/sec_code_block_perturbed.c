if (length < sizeof (dsth))
	{
	  fputc ('\n', file);
	  break;
	}
      dst_size -= length;
      off += length;
      length -= sizeof (dsth);
      if (length == 0)
	buf = NULL;
      else
	{
	  buf = _bfd_malloc_and_read (abfd, length, length);
	  if (buf == NULL)
	    {
	      fprintf (file, _("cannot read DST symbol\n"));
	      return;
	    }
	}
      switch (type)
	{
	case DSC__K_DTYPE_V:
	case DSC__K_DTYPE_BU:
	case DSC__K_DTYPE_WU:
	case DSC__K_DTYPE_LU:
	case DSC__K_DTYPE_QU:
	case DSC__K_DTYPE_B:
	case DSC__K_DTYPE_W:
	case DSC__K_DTYPE_L:
	case DSC__K_DTYPE_Q:
	case DSC__K_DTYPE_F:
	case DSC__K_DTYPE_D:
	case DSC__K_DTYPE_FC:
	case DSC__K_DTYPE_DC:
	case DSC__K_DTYPE_T:
	case DSC__K_DTYPE_NU:
	case DSC__K_DTYPE_NL:
	case DSC__K_DTYPE_NLO:
	case DSC__K_DTYPE_NR:
	case DSC__K_DTYPE_NRO:
	case DSC__K_DTYPE_NZ:
	case DSC__K_DTYPE_P:
	case DSC__K_DTYPE_ZI:
	case DSC__K_DTYPE_ZEM:
	case DSC__K_DTYPE_DSC:
	case DSC__K_DTYPE_OU:
	case DSC__K_DTYPE_O:
	case DSC__K_DTYPE_G:
	case DSC__K_DTYPE_H:
	case DSC__K_DTYPE_GC:
	case DSC__K_DTYPE_HC:
	case DSC__K_DTYPE_CIT:
	case DSC__K_DTYPE_BPV:
	case DSC__K_DTYPE_BLV:
	case DSC__K_DTYPE_VU:
	case DSC__K_DTYPE_ADT:
	case DSC__K_DTYPE_VT:
	case DSC__K_DTYPE_T2:
	case DSC__K_DTYPE_VT2:
	  fprintf (file, _("standard data: %s\n"),
		   evax_bfd_get_dsc_name (type));
	  evax_bfd_print_valspec (buf, length, 4, file);
	  fprintf (file, _("    name: %.*s\n"), buf[5], buf + 6);
	  break;
	case DST__K_MODBEG:
	  {
	    struct vms_dst_modbeg *dst = (void *)buf;
	    const char *name = (const char *)buf + sizeof (*dst);

	    fprintf (file, _("modbeg\n"));
	    if (length < sizeof (*dst))
	      break;
	    /* xgettext:c-format */
	    fprintf (file, _("   flags: %d, language: %u, "
			     "major: %u, minor: %u\n"),
		     dst->flags,
		     (unsigned)bfd_getl32 (dst->language),
		     (unsigned)bfd_getl16 (dst->major),
		     (unsigned)bfd_getl16 (dst->minor));
	    length -= sizeof (*dst);
	    if (length > 0)
	      {
		int nlen = length - 1;
		fprintf (file, _("   module name: %.*s\n"),
			 name[0] > nlen ? nlen : name[0], name + 1);
		if (name[0] < nlen)
		  {
		    length -= name[0] + 1;
		    name += name[0] + 1;
		    nlen = length - 1;
		    fprintf (file, _("   compiler   : %.*s\n"),
			     name[0] > nlen ? nlen: name[0], name + 1);
		  }
	      }
	  }
	  break;
	case DST__K_MODEND:
	  fprintf (file, _("modend\n"));
	  break;
	case DST__K_RTNBEG:
	  {
	    struct vms_dst_rtnbeg *dst = (void *)buf;
	    const char *name = (const char *)buf + sizeof (*dst);

	    fputs (_("rtnbeg\n"), file);
	    if (length >= sizeof (*dst))
	      {
		/* xgettext:c-format */
		fprintf (file, _("    flags: %u, address: 0x%08x, "
				 "pd-address: 0x%08x\n"),
			 dst->flags,
			 (unsigned) bfd_getl32 (dst->address),
			 (unsigned) bfd_getl32 (dst->pd_address));
		length -= sizeof (*dst);
		if (length > 0)
		  {
		    int nlen = length - 1;
		    fprintf (file, _("    routine name: %.*s\n"),
			     name[0] > nlen ? nlen : name[0], name + 1);
		  }
	      }
	  }
	  break;
	case DST__K_RTNEND:
	  {
	    struct vms_dst_rtnend *dst = (void *)buf;

	    if (length >= sizeof (*dst))
	      fprintf (file, _("rtnend: size 0x%08x\n"),
		       (unsigned) bfd_getl32 (dst->size));
	  }
	  break;
	case DST__K_PROLOG:
	  {
	    struct vms_dst_prolog *dst = (void *)buf;

	    if (length >= sizeof (*dst))
	      /* xgettext:c-format */
	      fprintf (file, _("prolog: bkpt address 0x%08x\n"),
		       (unsigned) bfd_getl32 (dst->bkpt_addr));
	  }
	  break;
	case DST__K_EPILOG:
	  {
	    struct vms_dst_epilog *dst = (void *)buf;

	    if (length >= sizeof (*dst))
	      /* xgettext:c-format */
	      fprintf (file, _("epilog: flags: %u, count: %u\n"),
		       dst->flags, (unsigned) bfd_getl32 (dst->count));
	  }
	  break;
	case DST__K_BLKBEG:
	  {
	    struct vms_dst_blkbeg *dst = (void *)buf;
	    const char *name = (const char *)buf + sizeof (*dst);

	    if (length > sizeof (*dst))
	      {
		int nlen;
		length -= sizeof (*dst);
		nlen = length - 1;
		/* xgettext:c-format */
		fprintf (file, _("blkbeg: address: 0x%08x, name: %.*s\n"),
			 (unsigned) bfd_getl32 (dst->address),
			 name[0] > nlen ? nlen : name[0], name + 1);
	      }
	  }
	  break;
	case DST__K_BLKEND:
	  {
	    struct vms_dst_blkend *dst = (void *)buf;

	    if (length >= sizeof (*dst))
	      /* xgettext:c-format */
	      fprintf (file, _("blkend: size: 0x%08x\n"),
		       (unsigned) bfd_getl32 (dst->size));
	  }
	  break;
	case DST__K_TYPSPEC:
	  {
	    fprintf (file, _("typspec (len: %u)\n"), length);
	    if (length >= 1)
	      {
		int nlen = length - 1;
		fprintf (file, _("    name: %.*s\n"),
			 buf[0] > nlen ? nlen : buf[0], buf + 1);
		if (nlen > buf[0])
		  evax_bfd_print_typspec (buf + 1 + buf[0], length - (1 + buf[0]),
					  5, file);
	      }
	  }
	  break;
	case DST__K_SEPTYP:
	  {
	    if (length >= 6)
	      {
		fprintf (file, _("septyp, name: %.*s\n"),
			 buf[5] > length - 6 ? length - 6 : buf[5], buf + 6);
		evax_bfd_print_valspec (buf, length, 4, file);
	      }
	  }
	  break;
	case DST__K_RECBEG:
	  {
	    struct vms_dst_recbeg *recbeg = (void *)buf;
	    const char *name = (const char *)buf + sizeof (*recbeg);

	    if (length > sizeof (*recbeg))
	      {
		int nlen = length - sizeof (*recbeg) - 1;
		if (name[0] < nlen)
		  nlen = name[0];
		fprintf (file, _("recbeg: name: %.*s\n"), nlen, name + 1);
		evax_bfd_print_valspec (buf, length, 4, file);
		length -= 1 + nlen;
		if (length >= 4)
		  fprintf (file, _("    len: %u bits\n"),
			   (unsigned) bfd_getl32 (name + 1 + nlen));
	      }
	  }
	  break;
	case DST__K_RECEND:
	  fprintf (file, _("recend\n"));
	  break;
	case DST__K_ENUMBEG:
	  if (length >= 2)
	    /* xgettext:c-format */
	    fprintf (file, _("enumbeg, len: %u, name: %.*s\n"),
		     buf[0], buf[1] > length - 2 ? length - 2 : buf[1], buf + 2);
	  break;
	case DST__K_ENUMELT:
	  if (length >= 6)
	    {
	      fprintf (file, _("enumelt, name: %.*s\n"),
		       buf[5] > length - 6 ? length - 6 : buf[5], buf + 6);
	      evax_bfd_print_valspec (buf, length, 4, file);
	    }
	  break;
	case DST__K_ENUMEND:
	  fprintf (file, _("enumend\n"));
	  break;
	case DST__K_LABEL:
	  {
	    struct vms_dst_label *lab = (void *)buf;
	    if (length >= sizeof (*lab))
	      {
		fprintf (file, _("label, name: %.*s\n"),
			 lab->name[0] > length - 1 ? length - 1 : lab->name[0],
			 lab->name + 1);
		fprintf (file, _("    address: 0x%08x\n"),
			 (unsigned) bfd_getl32 (lab->value));
	      }
	  }
	  break;
	case DST__K_DIS_RANGE:
	  if (length >= 4)
	    {
	      unsigned int cnt = bfd_getl32 (buf);
	      unsigned char *rng = buf + 4;
	      unsigned int i;

	      fprintf (file, _("discontiguous range (nbr: %u)\n"), cnt);
	      length -= 4;
	      for (i = 0; i < cnt; i++, rng += 8)
		{
		  if (length < 8)
		    break;
		  /* xgettext:c-format */
		  fprintf (file, _("    address: 0x%08x, size: %u\n"),
			   (unsigned) bfd_getl32 (rng),
			   (unsigned) bfd_getl32 (rng + 4));
		  length -= 8;
		}
	    }
	  break;
	case DST__K_LINE_NUM:
	  {
	    unsigned char *buf_orig = buf;

	    fprintf (file, _("line num  (len: %u)\n"), length);

	    while (length > 0)
	      {
		int cmd;
		unsigned int val;
		int cmdlen = -1;

		cmd = *buf++;
		length--;

		fputs ("    ", file);

		switch (cmd)
		  {
		  case DST__K_DELTA_PC_W:
		    if (length < 2)
		      break;
		    val = bfd_getl16 (buf);
		    fprintf (file, _("delta_pc_w %u\n"), val);
		    pc += val;
		    line++;
		    cmdlen = 2;
		    break;
		  case DST__K_INCR_LINUM:
		    if (length < 1)
		      break;
		    val = *buf;
		    fprintf (file, _("incr_linum(b): +%u\n"), val);
		    line += val;
		    cmdlen = 1;
		    break;
		  case DST__K_INCR_LINUM_W:
		    if (length < 2)
		      break;
		    val = bfd_getl16 (buf);
		    fprintf (file, _("incr_linum_w: +%u\n"), val);
		    line += val;
		    cmdlen = 2;
		    break;
		  case DST__K_INCR_LINUM_L:
		    if (length < 4)
		      break;
		    val = bfd_getl32 (buf);
		    fprintf (file, _("incr_linum_l: +%u\n"), val);
		    line += val;
		    cmdlen = 4;
		    break;
		  case DST__K_SET_LINUM:
		    if (length < 2)
		      break;
		    line = bfd_getl16 (buf);
		    fprintf (file, _("set_line_num(w) %u\n"), line);
		    cmdlen = 2;
		    break;
		  case DST__K_SET_LINUM_B:
		    if (length < 1)
		      break;
		    line = *buf;
		    fprintf (file, _("set_line_num_b %u\n"), line);
		    cmdlen = 1;
		    break;
		  case DST__K_SET_LINUM_L:
		    if (length < 4)
		      break;
		    line = bfd_getl32 (buf);
		    fprintf (file, _("set_line_num_l %u\n"), line);
		    cmdlen = 4;
		    break;
		  case DST__K_SET_ABS_PC:
		    if (length < 4)
		      break;
		    pc = bfd_getl32 (buf);
		    fprintf (file, _("set_abs_pc: 0x%08x\n"), pc);
		    cmdlen = 4;
		    break;
		  case DST__K_DELTA_PC_L:
		    if (length < 4)
		      break;
		    fprintf (file, _("delta_pc_l: +0x%08x\n"),
			     (unsigned) bfd_getl32 (buf));
		    cmdlen = 4;
		    break;
		  case DST__K_TERM:
		    if (length < 1)
		      break;
		    fprintf (file, _("term(b): 0x%02x"), *buf);
		    pc += *buf;
		    fprintf (file, _("        pc: 0x%08x\n"), pc);
		    cmdlen = 1;
		    break;
		  case DST__K_TERM_W:
		    if (length < 2)
		      break;
		    val = bfd_getl16 (buf);
		    fprintf (file, _("term_w: 0x%04x"), val);
		    pc += val;
		    fprintf (file, _("    pc: 0x%08x\n"), pc);
		    cmdlen = 2;
		    break;
		  default:
		    if (cmd <= 0)
		      {
			fprintf (file, _("delta pc +%-4d"), -cmd);
			line++;  /* FIXME: curr increment.  */
			pc += -cmd;
			/* xgettext:c-format */
			fprintf (file, _("    pc: 0x%08x line: %5u\n"),
				 pc, line);
			cmdlen = 0;
		      }
		    else
		      fprintf (file, _("    *unhandled* cmd %u\n"), cmd);
		    break;
		  }
		if (cmdlen < 0)
		  break;
		length -= cmdlen;
		buf += cmdlen;
	      }
	    buf = buf_orig;
	  }
	  break;
	case DST__K_SOURCE:
	  {
	    unsigned char *buf_orig = buf;

	    fprintf (file, _("source (len: %u)\n"), length);

	    while (length > 0)
	      {
		int cmd = *buf++;
		int cmdlen = -1;

		length--;
		switch (cmd)
		  {
		  case DST__K_SRC_DECLFILE:
		    {
		      struct vms_dst_src_decl_src *src = (void *) buf;
		      const char *name;
		      int nlen;

		      if (length < sizeof (*src))
			break;
		      /* xgettext:c-format */
		      fprintf (file, _("   declfile: len: %u, flags: %u, "
				       "fileid: %u\n"),
			       src->length, src->flags,
			       (unsigned)bfd_getl16 (src->fileid));
		      /* xgettext:c-format */
		      fprintf (file, _("   rms: cdt: 0x%08x %08x, "
				       "ebk: 0x%08x, ffb: 0x%04x, "
				       "rfo: %u\n"),
			       (unsigned)bfd_getl32 (src->rms_cdt + 4),
			       (unsigned)bfd_getl32 (src->rms_cdt + 0),
			       (unsigned)bfd_getl32 (src->rms_ebk),
			       (unsigned)bfd_getl16 (src->rms_ffb),
			       src->rms_rfo);
		      if (src->length > length || src->length <= sizeof (*src))
			break;
		      nlen = src->length - sizeof (*src) - 1;
		      name = (const char *) buf + sizeof (*src);
		      fprintf (file, _("   filename   : %.*s\n"),
			       name[0] > nlen ? nlen : name[0], name + 1);
		      if (name[0] >= nlen)
			break;
		      nlen -= name[0] + 1;
		      name += name[0] + 1;
		      fprintf (file, _("   module name: %.*s\n"),
			       name[0] > nlen ? nlen : name[0], name + 1);
		      if (name[0] > nlen)
			break;
		      cmdlen = src->length;
		    }
		    break;
		  case DST__K_SRC_SETFILE:
		    if (length < 2)
		      break;
		    fprintf (file, _("   setfile %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_SETREC_W:
		    if (length < 2)
		      break;
		    fprintf (file, _("   setrec %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_SETREC_L:
		    if (length < 4)
		      break;
		    fprintf (file, _("   setrec %u\n"),
			     (unsigned) bfd_getl32 (buf));
		    cmdlen = 4;
		    break;
		  case DST__K_SRC_SETLNUM_W:
		    if (length < 2)
		      break;
		    fprintf (file, _("   setlnum %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_SETLNUM_L:
		    if (length < 4)
		      break;
		    fprintf (file, _("   setlnum %u\n"),
			     (unsigned) bfd_getl32 (buf));
		    cmdlen = 4;
		    break;
		  case DST__K_SRC_DEFLINES_W:
		    if (length < 2)
		      break;
		    fprintf (file, _("   deflines %u\n"),
			     (unsigned) bfd_getl16 (buf));
		    cmdlen = 2;
		    break;
		  case DST__K_SRC_DEFLINES_B:
		    if (length < 1)
		      break;
		    fprintf (file, _("   deflines %u\n"), *buf);
		    cmdlen = 1;
		    break;
		  case DST__K_SRC_FORMFEED:
		    fprintf (file, _("   formfeed\n"));
		    cmdlen = 0;
		    break;
		  default:
		    fprintf (file, _("   *unhandled* cmd %u\n"), cmd);
		    break;
		  }
		if (cmdlen < 0)
		  break;
		length -= cmdlen;
		buf += cmdlen;
	      }
	    buf = buf_orig;
	  }
	  break;
	default:
	  fprintf (file, _("*unhandled* dst type %u\n"), type);
	  break;
	}