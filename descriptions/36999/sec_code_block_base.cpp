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