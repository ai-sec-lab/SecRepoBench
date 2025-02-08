void TableLookUp::setTable(int ntable, const std::vector<ushort16>& lookuptable) {
  assert(!lookuptable.empty());

  const int nfilled = lookuptable.size();
  if (nfilled >= 65536)
    ThrowRDE("Table lookup with %i entries is unsupported", nfilled);

  if (ntable > ntables) {
    ThrowRDE("Table lookup with number greater than number of tables.");
  }
  ushort16* t = &tables[ntable * TABLE_SIZE];
  if (!dither) {
    for (int i = 0; i < 65536; i++) {
      t[i] = (i < nfilled) ? lookuptable[i] : lookuptable[nfilled - 1];
    }
    return;
  }
  for (int i = 0; i < nfilled; i++) {
    int center = lookuptable[i];
    int lower = i > 0 ? lookuptable[i - 1] : center;
    int upper = i < (nfilled - 1) ? lookuptable[i + 1] : center;
    int delta = upper - lower;
    t[i * 2] = center - ((upper - lower + 2) / 4);
    t[i * 2 + 1] = delta;
  }

  for (int i = nfilled; i < 65536; i++) {
    t[i * 2] = lookuptable[nfilled - 1];
    t[i * 2 + 1] = 0;
  }
  t[0] = t[1];
  t[TABLE_SIZE - 1] = t[TABLE_SIZE - 2];
}