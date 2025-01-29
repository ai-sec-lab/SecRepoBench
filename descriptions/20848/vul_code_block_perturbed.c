struct bfd_preserve preserve;

  if (matching != NULL)
    *matching = NULL;

  if (!bfd_read_p (abfd)
      || (unsigned int) abfd->format >= (unsigned int) bfd_type_end)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  if (abfd->format != bfd_unknown)
    return abfd->format == format;

  if (matching != NULL || *bfd_associated_vector != NULL)
    {
      bfd_size_type amt;

      amt = sizeof (*matching_vector) * 2 * _bfd_target_vector_entries;
      matching_vector = (const bfd_target **) bfd_malloc (amt);
      if (!matching_vector)
	return FALSE;
    }

  /* Presume the answer is yes.  */
  abfd->format = format;
  save_targ = abfd->xvec;
  preserve.marker = NULL;

  /* If the target type was explicitly specified, just check that target.  */
  if (!abfd->target_defaulted)
    {
      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)	/* rewind! */
	goto err_ret;

      right_targ = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));

      if (right_targ)
	goto ok_ret;

      /* For a long time the code has dropped through to check all
	 targets if the specified target was wrong.  I don't know why,
	 and I'm reluctant to change it.  However, in the case of an
	 archive, it can cause problems.  If the specified target does
	 not permit archives (e.g., the binary target), then we should
	 not allow some other target to recognize it as an archive, but
	 should instead allow the specified target to recognize it as an
	 object.  When I first made this change, it broke the PE target,
	 because the specified pei-i386 target did not recognize the
	 actual pe-i386 archive.  Since there may be other problems of
	 this sort, I changed this test to check only for the binary
	 target.  */
      if (format == bfd_archive && save_targ == &binary_vec)
	goto err_unrecog;
    }

  /* Since the target type was defaulted, check them all in the hope
     that one will be uniquely recognized.  */
  right_targ = NULL;
  ar_right_targ = NULL;
  match_targ = NULL;
  best_match = 256;
  optimalcount = 0;
  match_count = 0;
  ar_match_index = _bfd_target_vector_entries;

  for (target = bfd_target_vector; *target != NULL; target++)
    {
      const bfd_target *temp;

      /* The binary target matches anything, so don't return it when
	 searching.  Don't match the plugin target if we have another
	 alternative since we want to properly set the input format
	 before allowing a plugin to claim the file.  Also, don't
	 check the default target twice.  */
      if (*target == &binary_vec
#if BFD_SUPPORTS_PLUGINS
	  || (match_count != 0 && *target == &plugin_vec)
#endif
	  || (!abfd->target_defaulted && *target == save_targ))
	continue;

      /* If we already tried a match, the bfd is modified and may
	 have sections attached, which will confuse the next
	 _bfd_check_format call.  */
      bfd_reinit (abfd, initial_section_id);

      /* Change BFD's target temporarily.  */
      abfd->xvec = *target;

      if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
	goto err_ret;

      /* If _bfd_check_format neglects to set bfd_error, assume
	 bfd_error_wrong_format.  We didn't used to even pay any
	 attention to bfd_error, so I suspect that some
	 _bfd_check_format might have this problem.  */
      bfd_set_error (bfd_error_wrong_format);

      temp = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));
      if (temp)
	{
	  int match_priority = temp->match_priority;
#if BFD_SUPPORTS_PLUGINS
	  /* If this object can be handled by a plugin, give that the
	     lowest priority; objects both handled by a plugin and
	     with an underlying object format will be claimed
	     separately by the plugin.  */
	  if (*target == &plugin_vec)
	    match_priority = (*target)->match_priority;
#endif

	  match_targ = temp;
	  if (preserve.marker != NULL)
	    bfd_preserve_finish (abfd, &preserve);

	  if (abfd->format != bfd_archive
	      || (bfd_has_map (abfd)
		  && bfd_get_error () != bfd_error_wrong_object_format))
	    {
	      /* If this is the default target, accept it, even if
		 other targets might match.  People who want those
		 other targets have to set the GNUTARGET variable.  */
	      if (temp == bfd_default_vector[0])
		goto ok_ret;

	      if (matching_vector)
		matching_vector[match_count] = temp;
	      match_count++;

	      if (match_priority < best_match)
		{
		  best_match = match_priority;
		  optimalcount = 0;
		}
	      if (match_priority <= best_match)
		{
		  /* This format checks out as ok!  */
		  right_targ = temp;
		  optimalcount++;
		}
	    }
	  else
	    {
	      /* An archive with no armap or objects of the wrong
		 type.  We want this target to match if we get no
		 better matches.  */
	      if (ar_right_targ != bfd_default_vector[0])
		ar_right_targ = *target;
	      if (matching_vector)
		matching_vector[ar_match_index] = *target;
	      ar_match_index++;
	    }

	  if (!bfd_preserve_save (abfd, &preserve))
	    goto err_ret;
	}
      else if (bfd_get_error () != bfd_error_wrong_format)
	goto err_ret;
    }

  if (optimalcount == 1)
    match_count = 1;

  if (match_count == 0)
    {
      /* Try partial matches.  */
      right_targ = ar_right_targ;

      if (right_targ == bfd_default_vector[0])
	{
	  match_count = 1;
	}
      else
	{
	  match_count = ar_match_index - _bfd_target_vector_entries;

	  if (matching_vector && match_count > 1)
	    memcpy (matching_vector,
		    matching_vector + _bfd_target_vector_entries,
		    sizeof (*matching_vector) * match_count);
	}
    }

  /* We have more than one equally good match.  If any of the best
     matches is a target in config.bfd targ_defvec or targ_selvecs,
     choose it.  */
  if (match_count > 1)
    {
      const bfd_target * const *assoc = bfd_associated_vector;

      while ((right_targ = *assoc++) != NULL)
	{
	  int i = match_count;

	  while (--i >= 0)
	    if (matching_vector[i] == right_targ
		&& right_targ->match_priority <= best_match)
	      break;

	  if (i >= 0)
	    {
	      match_count = 1;
	      break;
	    }
	}
    }

  /* We still have more than one equally good match, and at least some
     of the targets support match priority.  Choose the first of the
     best matches.  */
  if (matching_vector && match_count > 1 && optimalcount != match_count)
    {
      int i;

      for (i = 0; i < match_count; i++)
	{
	  right_targ = matching_vector[i];
	  if (right_targ->match_priority <= best_match)
	    break;
	}
      match_count = 1;
    }

  /* There is way too much undoing of half-known state here.  We
     really shouldn't iterate on live bfd's.  Note that saving the
     whole bfd and restoring it would be even worse; the first thing
     you notice is that the cached bfd file position gets out of sync.  */
  if (preserve.marker != NULL)
    bfd_preserve_restore (abfd, &preserve);

  if (match_count == 1)
    {
      abfd->xvec = right_targ;
      /* If we come out of the loop knowing that the last target that
	 matched is the one we want, then ABFD should still be in a usable
	 state (except possibly for XVEC).  */
      if (match_targ != right_targ)
	{
	  bfd_reinit (abfd, initial_section_id);
	  if (bfd_seek (abfd, (file_ptr) 0, SEEK_SET) != 0)
	    goto err_ret;
	  match_targ = BFD_SEND_FMT (abfd, _bfd_check_format, (abfd));
	  BFD_ASSERT (match_targ != NULL);
	}

    ok_ret:
      /* If the file was opened for update, then `output_has_begun'
	 some time ago when the file was created.  Do not recompute
	 sections sizes or alignments in _bfd_set_section_contents.
	 We can not set this flag until after checking the format,
	 because it will interfere with creation of BFD sections.  */
      if (abfd->direction == both_direction)
	abfd->output_has_begun = TRUE;

      if (matching_vector)
	free (matching_vector);

      /* File position has moved, BTW.  */
      return TRUE;
    }

  if (match_count == 0)
    {
    err_unrecog:
      bfd_set_error (bfd_error_file_not_recognized);
    err_ret:
      abfd->xvec = save_targ;
      abfd->format = bfd_unknown;
      if (matching_vector)
	free (matching_vector);
      if (preserve.marker != NULL)
	bfd_preserve_restore (abfd, &preserve);
      return FALSE;
    }

  /* Restore original target type and format.  */
  abfd->xvec = save_targ;
  abfd->format = bfd_unknown;
  bfd_set_error (bfd_error_file_ambiguously_recognized);

  if (matching)
    {
      *matching = (char **) matching_vector;
      matching_vector[match_count] = NULL;
      /* Return target names.  This is a little nasty.  Maybe we
	 should do another bfd_malloc?  */
      while (--match_count >= 0)
	{
	  const char *name = matching_vector[match_count]->name;
	  *(const char **) &matching_vector[match_count] = name;
	}
    }
  else if (matching_vector)
    free (matching_vector);