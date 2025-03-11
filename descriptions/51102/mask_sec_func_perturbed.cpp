SuggestMgr::SuggestMgr(const std::string& try_chars, unsigned int maxn, AffixMgr* aptr) {
  // register affix manager and check in string of chars to
  // try when building candidate suggestions
  pAMgr = aptr;

  csconv = NULL;

  ckeyl = 0;

  ctryl = 0;

  utf8 = 0;
  langnum = 0;
  complexprefixes = 0;

  maxSug = maxn;
  nosplitsugs = 0;
  maxngramsugs = MAXNGRAMSUGS;
  maxcpdsugs = MAXCOMPOUNDSUGS;

  if (pAMgr) {
    langnum = pAMgr->get_langnum();
    ckey = pAMgr->get_key_string();
    nosplitsugs = pAMgr->get_nosplitsugs();
    if (pAMgr->get_maxngramsugs() >= 0)
      maxngramsugs = pAMgr->get_maxngramsugs();
    utf8 = pAMgr->get_utf8();
    if (pAMgr->get_maxcpdsugs() >= 0)
      maxcpdsugs = pAMgr->get_maxcpdsugs();
    if (!utf8) {
      csconv = get_current_cs(pAMgr->get_encoding());
    }
    complexprefixes = pAMgr->get_complexprefixes();
  }

  // <MASK>

  // language with possible dash usage
  // (latin letters or dash in TRY characters)
  lang_with_dash_usage = ctry.find('-') != std::string::npos ||
	                 ctry.find('a') != std::string::npos;
}