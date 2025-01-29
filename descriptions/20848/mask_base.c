/* Generic BFD support for file formats.
   Copyright (C) 1990-2020 Free Software Foundation, Inc.
   Written by Cygnus Support.

   This file is part of BFD, the Binary File Descriptor library.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */


/*
SECTION
	File formats

	A format is a BFD concept of high level file contents type. The
	formats supported by BFD are:

	o <<bfd_object>>

	The BFD may contain data, symbols, relocations and debug info.

	o <<bfd_archive>>

	The BFD contains other BFDs and an optional index.

	o <<bfd_core>>

	The BFD contains the result of an executable core dump.

SUBSECTION
	File format functions
*/

#include "sysdep.h"
#include "bfd.h"
#include "libbfd.h"

/* IMPORT from targets.c.  */
extern const size_t _bfd_target_vector_entries;

/*
FUNCTION
	bfd_check_format

SYNOPSIS
	bfd_boolean bfd_check_format (bfd *abfd, bfd_format format);

DESCRIPTION
	Verify if the file attached to the BFD @var{abfd} is compatible
	with the format @var{format} (i.e., one of <<bfd_object>>,
	<<bfd_archive>> or <<bfd_core>>).

	If the BFD has been set to a specific target before the
	call, only the named target and format combination is
	checked. If the target has not been set, or has been set to
	<<default>>, then all the known target backends is
	interrogated to determine a match.  If the default target
	matches, it is used.  If not, exactly one target must recognize
	the file, or an error results.

	The function returns <<TRUE>> on success, otherwise <<FALSE>>
	with one of the following error codes:

	o <<bfd_error_invalid_operation>> -
	if <<format>> is not one of <<bfd_object>>, <<bfd_archive>> or
	<<bfd_core>>.

	o <<bfd_error_system_call>> -
	if an error occured during a read - even some file mismatches
	can cause bfd_error_system_calls.

	o <<file_not_recognised>> -
	none of the backends recognised the file format.

	o <<bfd_error_file_ambiguously_recognized>> -
	more than one backend recognised the file format.
*/

bfd_boolean
bfd_check_format (bfd *abfd, bfd_format format)
{
  return bfd_check_format_matches (abfd, format, NULL);
}

struct bfd_preserve
{
  void *marker;
  void *tdata;
  flagword flags;
  const struct bfd_arch_info *arch_info;
  struct bfd_section *sections;
  struct bfd_section *section_last;
  unsigned int section_count;
  unsigned int section_id;
  struct bfd_hash_table section_htab;
  const struct bfd_build_id *build_id;
};

/* When testing an object for compatibility with a particular target
   back-end, the back-end object_p function needs to set up certain
   fields in the bfd on successfully recognizing the object.  This
   typically happens in a piecemeal fashion, with failures possible at
   many points.  On failure, the bfd is supposed to be restored to its
   initial state, which is virtually impossible.  However, restoring a
   subset of the bfd state works in practice.  This function stores
   the subset.  */

static bfd_boolean
bfd_preserve_save (bfd *abfd, struct bfd_preserve *preserve)
{
  preserve->tdata = abfd->tdata.any;
  preserve->arch_info = abfd->arch_info;
  preserve->flags = abfd->flags;
  preserve->sections = abfd->sections;
  preserve->section_last = abfd->section_last;
  preserve->section_count = abfd->section_count;
  preserve->section_id = _bfd_section_id;
  preserve->section_htab = abfd->section_htab;
  preserve->marker = bfd_alloc (abfd, 1);
  preserve->build_id = abfd->build_id;
  if (preserve->marker == NULL)
    return FALSE;

  return bfd_hash_table_init (&abfd->section_htab, bfd_section_hash_newfunc,
			      sizeof (struct section_hash_entry));
}

/* Clear out a subset of BFD state.  */

static void
bfd_reinit (bfd *abfd, unsigned int section_id)
{
  abfd->tdata.any = NULL;
  abfd->arch_info = &bfd_default_arch_struct;
  abfd->flags &= BFD_FLAGS_SAVED;
  bfd_section_list_clear (abfd);
  _bfd_section_id = section_id;
}

/* Restores bfd state saved by bfd_preserve_save.  */

static void
bfd_preserve_restore (bfd *abfd, struct bfd_preserve *preserve)
{
  bfd_hash_table_free (&abfd->section_htab);

  abfd->tdata.any = preserve->tdata;
  abfd->arch_info = preserve->arch_info;
  abfd->flags = preserve->flags;
  abfd->section_htab = preserve->section_htab;
  abfd->sections = preserve->sections;
  abfd->section_last = preserve->section_last;
  abfd->section_count = preserve->section_count;
  _bfd_section_id = preserve->section_id;
  abfd->build_id = preserve->build_id;

  /* bfd_release frees all memory more recently bfd_alloc'd than
     its arg, as well as its arg.  */
  bfd_release (abfd, preserve->marker);
  preserve->marker = NULL;
}

/* Called when the bfd state saved by bfd_preserve_save is no longer
   needed.  */

static void
bfd_preserve_finish (bfd *abfd ATTRIBUTE_UNUSED, struct bfd_preserve *preserve)
{
  /* It would be nice to be able to free more memory here, eg. old
     tdata, but that's not possible since these blocks are sitting
     inside bfd_alloc'd memory.  The section hash is on a separate
     objalloc.  */
  bfd_hash_table_free (&preserve->section_htab);
  preserve->marker = NULL;
}

/*
FUNCTION
	bfd_check_format_matches

SYNOPSIS
	bfd_boolean bfd_check_format_matches
	  (bfd *abfd, bfd_format format, char ***matching);

DESCRIPTION
	Like <<bfd_check_format>>, except when it returns FALSE with
	<<bfd_errno>> set to <<bfd_error_file_ambiguously_recognized>>.  In that
	case, if @var{matching} is not NULL, it will be filled in with
	a NULL-terminated list of the names of the formats that matched,
	allocated with <<malloc>>.
	Then the user may choose a format and try again.

	When done with the list that @var{matching} points to, the caller
	should free it.
*/

bfd_boolean
bfd_check_format_matches (bfd *abfd, bfd_format format, char ***matching)
{
  extern const bfd_target binary_vec;
#if BFD_SUPPORTS_PLUGINS
  extern const bfd_target plugin_vec;
#endif
  const bfd_target * const *target;
  const bfd_target **matching_vector = NULL;
  const bfd_target *save_targ, *right_targ, *ar_right_targ, *match_targ;
  int match_count, best_count, best_match;
  int ar_match_index;
  unsigned int initial_section_id = _bfd_section_id;
  // <MASK>
  return FALSE;
}

/*
FUNCTION
	bfd_set_format

SYNOPSIS
	bfd_boolean bfd_set_format (bfd *abfd, bfd_format format);

DESCRIPTION
	This function sets the file format of the BFD @var{abfd} to the
	format @var{format}. If the target set in the BFD does not
	support the format requested, the format is invalid, or the BFD
	is not open for writing, then an error occurs.
*/

bfd_boolean
bfd_set_format (bfd *abfd, bfd_format format)
{
  if (bfd_read_p (abfd)
      || (unsigned int) abfd->format >= (unsigned int) bfd_type_end)
    {
      bfd_set_error (bfd_error_invalid_operation);
      return FALSE;
    }

  if (abfd->format != bfd_unknown)
    return abfd->format == format;

  /* Presume the answer is yes.  */
  abfd->format = format;

  if (!BFD_SEND_FMT (abfd, _bfd_set_format, (abfd)))
    {
      abfd->format = bfd_unknown;
      return FALSE;
    }

  return TRUE;
}

/*
FUNCTION
	bfd_format_string

SYNOPSIS
	const char *bfd_format_string (bfd_format format);

DESCRIPTION
	Return a pointer to a const string
	<<invalid>>, <<object>>, <<archive>>, <<core>>, or <<unknown>>,
	depending upon the value of @var{format}.
*/

const char *
bfd_format_string (bfd_format format)
{
  if (((int) format < (int) bfd_unknown)
      || ((int) format >= (int) bfd_type_end))
    return "invalid";

  switch (format)
    {
    case bfd_object:
      return "object";		/* Linker/assembler/compiler output.  */
    case bfd_archive:
      return "archive";		/* Object archive file.  */
    case bfd_core:
      return "core";		/* Core dump.  */
    default:
      return "unknown";
    }
}
