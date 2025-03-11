bool AffixMgr::parse_phonetable(const std::string& line, FileMgr* fileMgr) {
  if (phone) {
    HUNSPELL_WARNING(stderr, "error: line %d: multiple table definitions\n",
                     fileMgr->getlinenum());
    return false;
  }
  // <MASK>
  return true;
}