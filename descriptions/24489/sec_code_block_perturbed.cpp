if (unlikely (!privateDicts.resize (fdCount)))
      { fini (); return; }
      for (unsigned int i = 0; i < fdCount; i++)
	privateDicts[i].init ();

      // parse CID font dicts and gather private dicts
      if (is_CID ())
      {
	for (unsigned int i = 0; i < fdCount; i++)
	{
	  byte_str_t fontDictStr = (*fdArray)[i];
	  if (unlikely (!fontDictStr.sanitize (&sc))) { fini (); return; }
	  cff1_font_dict_values_t *font;
	  cff1_font_dict_interpreter_t font_interp;
	  font_interp.env.init (fontDictStr);
	  font = fontDicts.push ();
	  if (unlikely (font == &Crap (cff1_font_dict_values_t))) { fini (); return; }
	  font->init ();
	  if (unlikely (!font_interp.interpret (*font))) { fini (); return; }
	  PRIVDICTVAL *priv = &privateDicts[i];
	  const byte_str_t privDictStr (StructAtOffset<UnsizedByteStr> (cff, font->privateDictInfo.offset), font->privateDictInfo.size);
	  if (unlikely (!privDictStr.sanitize (&sc))) { fini (); return; }
	  dict_interpreter_t<PRIVOPSET, PRIVDICTVAL> priv_interp;
	  priv_interp.env.init (privDictStr);
	  priv->init ();
	  if (unlikely (!priv_interp.interpret (*priv))) { fini (); return; }

	  priv->localSubrs = &StructAtOffsetOrNull<CFF1Subrs> (&privDictStr, priv->subrsOffset);
	  if (priv->localSubrs != &Null (CFF1Subrs) &&
	      unlikely (!priv->localSubrs->sanitize (&sc)))
	  { fini (); return; }
	}
      }
      else  /* non-CID */
      {
	cff1_top_dict_values_t *font = &topDict;
	PRIVDICTVAL *priv = &privateDicts[0];

	const byte_str_t privDictStr (StructAtOffset<UnsizedByteStr> (cff, font->privateDictInfo.offset), font->privateDictInfo.size);
	if (unlikely (!privDictStr.sanitize (&sc))) { fini (); return; }
	dict_interpreter_t<PRIVOPSET, PRIVDICTVAL> priv_interp;
	priv_interp.env.init (privDictStr);
	priv->init ();
	if (unlikely (!priv_interp.interpret (*priv))) { fini (); return; }

	priv->localSubrs = &StructAtOffsetOrNull<CFF1Subrs> (&privDictStr, priv->subrsOffset);
	if (priv->localSubrs != &Null (CFF1Subrs) &&
	    unlikely (!priv->localSubrs->sanitize (&sc)))
	{ fini (); return; }
      }