bool find_table_index (hb_tag_t tag, unsigned int *table_index) const
  {
    Tag t;
    t = tag;
    return tables.bfind (t, table_index, HB_NOT_FOUND_STORE, Index::NOT_FOUND_INDEX);
  }
  const TableRecord& get_table_by_tag (hb_tag_t tag) const
  {
    unsigned int table_index;
    find_table_index (tag, &table_index);
    return get_table (table_index);
  }