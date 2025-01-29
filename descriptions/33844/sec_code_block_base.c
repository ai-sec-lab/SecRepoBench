if (koff > sizeof (kblk) - sizeof (struct vms_kbn))
		    return false;
		  kbn = (struct vms_kbn *)(kblk + koff);
		  klen = bfd_getl16 (kbn->keylen);
		  if (klen > sizeof (kblk) - sizeof (struct vms_kbn) - koff)
		    return false;
		  kvbn = bfd_getl32 (kbn->rfa.vbn);
		  koff = bfd_getl16 (kbn->rfa.offset);