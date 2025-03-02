int map (int value, unsigned int from_offset = 0, unsigned int to_offset = 1) const
  {
#define fromCoord coords[from_offset]
#define toCoord coords[to_offset]
    /* The following special-cases are not part of OpenType, which requires
     * that at least -1, 0, and +1 must be mapped. But we include these as
     * part of a better error recovery scheme. */
    if (len < 2)
    {
      if (!len)
	return value;
      else /* len == 1*/
	return value - arrayZ[0].fromCoord + arrayZ[0].toCoord;
    }

    // <MASK>
    return arrayZ[i-1].toCoord +
	   ((arrayZ[i].toCoord - arrayZ[i-1].toCoord) *
	    (value - arrayZ[i-1].fromCoord) + denom/2) / denom;
#undef toCoord
#undef fromCoord
  }