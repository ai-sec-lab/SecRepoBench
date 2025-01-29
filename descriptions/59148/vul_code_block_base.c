(r->howto->rightshift != 0
		  || bfd_get_reloc_size (r->howto) != 4
		  || r->howto->bitsize != 32
		  || r->howto->pc_relative
		  || r->howto->bitpos != 0
		  || r->howto->dst_mask != 0xffffffff
		  || octets + 4 > stabsize)
		{
		  _bfd_error_handler
		    (_("unsupported .stab relocation"));
		  bfd_set_error (bfd_error_invalid_operation);
		  goto out3;
		}