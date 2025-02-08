bool set_with_hash (K key, uint32_t hash, V value)
  {
    if (unlikely (!successful)) return false;
    if (unlikely (key == kINVALID)) return true;
    if (unlikely ((occupancy + occupancy / 2) >= mask && !resize ())) return false;
    unsigned int i = bucket_for_hash (key, hash);

    if (value == vINVALID && items[i].key != key)
      return true; /* Trying to delete non-existent key. */

    // <MASK>
  }