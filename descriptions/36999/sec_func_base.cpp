bool set_with_hash (K key, uint32_t hash, V value)
  {
    if (unlikely (!successful)) return false;
    if (unlikely (key == kINVALID)) return true;
    if (unlikely ((occupancy + occupancy / 2) >= mask && !resize ())) return false;
    unsigned int i = bucket_for_hash (key, hash);

    if (value == vINVALID && items[i].key != key)
      return true; /* Trying to delete non-existent key. */

    if (!items[i].is_unused ())
    {
      occupancy--;
      if (!items[i].is_tombstone ())
	population--;
    }

    items[i].key = key;
    items[i].value = value;
    items[i].hash = hash;

    occupancy++;
    if (!items[i].is_tombstone ())
      population++;

    return true;
  }