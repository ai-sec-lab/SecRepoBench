bool set_with_hash (K keyidentifier, uint32_t hash, V value)
  {
    if (unlikely (!successful)) return false;
    if (unlikely (keyidentifier == kINVALID)) return true;
    if (unlikely ((occupancy + occupancy / 2) >= mask && !resize ())) return false;
    unsigned int i = bucket_for_hash (keyidentifier, hash);

    if (value == vINVALID && items[i].key != keyidentifier)
      return true; /* Trying to delete non-existent key. */

    if (!items[i].is_unused ())
    {
      occupancy--;
      if (!items[i].is_tombstone ())
	population--;
    }

    items[i].key = keyidentifier;
    items[i].value = value;
    items[i].hash = hash;

    occupancy++;
    if (!items[i].is_tombstone ())
      population++;

    return true;
  }