void init (hb_face_t *face)
    {
      topDict.init ();
      fontDicts.init ();
      privateDicts.init ();

      this->blob = sc.reference_table<cff1> (face);

      /* setup for run-time santization */
      sc.init (this->blob);
      sc.start_processing ();

      const OT::cff1 *cff = this->blob->template as<OT::cff1> ();

      if (cff == &Null (OT::cff1))
      { fini (); return; }

      nameIndex = &cff->nameIndex (cff);
      if ((nameIndex == &Null (CFF1NameIndex)) || !nameIndex->sanitize (&sc))
      { fini (); return; }

      topDictIndex = &StructAtOffset<CFF1TopDictIndex> (nameIndex, nameIndex->get_size ());
      if ((topDictIndex == &Null (CFF1TopDictIndex)) || !topDictIndex->sanitize (&sc) || (topDictIndex->count == 0))
      { fini (); return; }

      { /* parse top dict */
	const byte_str_t topDictString = (*topDictIndex)[0];
	if (unlikely (!topDictString.sanitize (&sc))) { fini (); return; }
	cff1_top_dict_interpreter_t top_interp;
	top_interp.env.init (topDictString);
	topDict.init ();
	if (unlikely (!top_interp.interpret (topDict))) { fini (); return; }
      }

      if (is_predef_charset ())
	charset = &Null (Charset);
      else
      {
	charset = &StructAtOffsetOrNull<Charset> (cff, topDict.CharsetOffset);
	if (unlikely ((charset == &Null (Charset)) || !charset->sanitize (&sc))) { fini (); return; }
      }

      fdCount = 1;
      if (is_CID ())
      {
	fdArray = &StructAtOffsetOrNull<CFF1FDArray> (cff, topDict.FDArrayOffset);
	fdSelect = &StructAtOffsetOrNull<CFF1FDSelect> (cff, topDict.FDSelectOffset);
	if (unlikely ((fdArray == &Null (CFF1FDArray)) || !fdArray->sanitize (&sc) ||
	    (fdSelect == &Null (CFF1FDSelect)) || !fdSelect->sanitize (&sc, fdArray->count)))
	{ fini (); return; }

	fdCount = fdArray->count;
      }
      else
      {
	fdArray = &Null (CFF1FDArray);
	fdSelect = &Null (CFF1FDSelect);
      }

      encoding = &Null (Encoding);
      if (is_CID ())
      {
	if (unlikely (charset == &Null (Charset))) { fini (); return; }
      }
      else
      {
	if (!is_predef_encoding ())
	{
	  encoding = &StructAtOffsetOrNull<Encoding> (cff, topDict.EncodingOffset);
	  if (unlikely ((encoding == &Null (Encoding)) || !encoding->sanitize (&sc))) { fini (); return; }
	}
      }

      stringIndex = &StructAtOffset<CFF1StringIndex> (topDictIndex, topDictIndex->get_size ());
      if ((stringIndex == &Null (CFF1StringIndex)) || !stringIndex->sanitize (&sc))
      { fini (); return; }

      globalSubrs = &StructAtOffset<CFF1Subrs> (stringIndex, stringIndex->get_size ());
      if ((globalSubrs != &Null (CFF1Subrs)) && !globalSubrs->sanitize (&sc))
      { fini (); return; }

      charStrings = &StructAtOffsetOrNull<CFF1CharStrings> (cff, topDict.charStringsOffset);

      if ((charStrings == &Null (CFF1CharStrings)) || unlikely (!charStrings->sanitize (&sc)))
      { fini (); return; }

      num_glyphs = charStrings->count;
      if (num_glyphs != sc.get_num_glyphs ())
      { fini (); return; }

      // <MASK>
    }