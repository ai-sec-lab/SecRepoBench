if (bfd_libdata (abfd)->ver == LBR_MAJORID)
	{
	  struct vms_idx *ridx = (struct vms_idx *)p;

	  idx_vbn = bfd_getl32 (ridx->rfa.vbn);
	  idx_off = bfd_getl16 (ridx->rfa.offset);

	  keylen = ridx->keylen;
	  flags = 0;
	  keyname = ridx->keyname;
	}
      else if (bfd_libdata (abfd)->ver == LBR_ELFMAJORID)
	{
	  struct vms_elfidx *ridx = (struct vms_elfidx *)p;

	  idx_vbn = bfd_getl32 (ridx->rfa.vbn);
	  idx_off = bfd_getl16 (ridx->rfa.offset);

	  keylen = bfd_getl16 (ridx->keylen);
	  flags = ridx->flags;
	  keyname = ridx->keyname;
	}
      else
	return FALSE;

      /* Illegal value.  */
      if (idx_vbn == 0)
	return FALSE;

      /* Point to the next index entry.  */
      p = keyname + keylen;
      if (p > endp)
	return FALSE;

      if (idx_off == RFADEF__C_INDEX)
	{
	  /* Indirect entry.  Recurse.  */
	  if (!vms_traverse_index (abfd, idx_vbn, cs, recur_count + 1))
	    return FALSE;
	}
      else
	{
	  /* Add a new entry.  */
	  char *name;

	  if (flags & ELFIDX__SYMESC)
	    {
	      /* Extended key name.  */
	      unsigned int noff = 0;
	      unsigned int koff;
	      unsigned int kvbn;
	      struct vms_kbn *kbn;
	      unsigned char kblk[VMS_BLOCK_SIZE];

	      /* Sanity check.  */
	      if (keylen != sizeof (struct vms_kbn))
		return FALSE;

	      kbn = (struct vms_kbn *)keyname;
	      keylen = bfd_getl16 (kbn->keylen);

	      name = bfd_alloc (abfd, keylen + 1);
	      if (name == NULL)
		return FALSE;
	      kvbn = bfd_getl32 (kbn->rfa.vbn);
	      koff = bfd_getl16 (kbn->rfa.offset);

	      /* Read the key, chunk by chunk.  */
	      do
		{
		  unsigned int klen;

		  if (!vms_read_block (abfd, kvbn, kblk))
		    return FALSE;
		  if (koff > sizeof (kblk) - sizeof (struct vms_kbn))
		    return FALSE;
		  kbn = (struct vms_kbn *)(kblk + koff);
		  klen = bfd_getl16 (kbn->keylen);
		  if (klen > sizeof (kblk) - koff)
		    return FALSE;
		  kvbn = bfd_getl32 (kbn->rfa.vbn);
		  koff = bfd_getl16 (kbn->rfa.offset);

		  if (noff + klen > keylen)
		    return FALSE;
		  memcpy (name + noff, kbn + 1, klen);
		  noff += klen;
		}
	      while (kvbn != 0);

	      /* Sanity check.  */
	      if (noff != keylen)
		return FALSE;
	    }
	  else
	    {
	      /* Usual key name.  */
	      name = bfd_alloc (abfd, keylen + 1);
	      if (name == NULL)
		return FALSE;

	      memcpy (name, keyname, keylen);
	    }
	  name[keylen] = 0;

	  if (flags & ELFIDX__LISTRFA)
	    {
	      struct vms_lhs lhs;

	      /* Read the LHS.  */
	      off = (idx_vbn - 1) * VMS_BLOCK_SIZE + idx_off;
	      if (bfd_seek (abfd, off, SEEK_SET) != 0
		  || bfd_bread (&lhs, sizeof (lhs), abfd) != sizeof (lhs))
		return FALSE;

	      /* These extra entries may cause reallocation of CS.  */
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.ng_g_rfa))
		return FALSE;
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.ng_wk_rfa))
		return FALSE;
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.g_g_rfa))
		return FALSE;
	      if (!vms_add_indexes_from_list (abfd, cs, name, &lhs.g_wk_rfa))
		return FALSE;
	    }
	  else
	    {
	      if (!vms_add_index (cs, name, idx_vbn, idx_off))
		return FALSE;
	    }
	}