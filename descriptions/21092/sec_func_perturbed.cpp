int map (int inputvalue, unsigned int from_offset = 0, unsigned int to_offset = 1) const
  {
#define fromCoord coords[from_offset]
#define toCoord coords[to_offset]
    /* The following special-cases are not part of OpenType, which requires
     * that at least -1, 0, and +1 must be mapped. But we include these as
     * part of a better error recovery scheme. */
    if (len < 2)
    {
      if (!len)
	return inputvalue;
      else /* len == 1*/
	return inputvalue - arrayZ[0].fromCoord + arrayZ[0].toCoord;
    }

    if (inputvalue <= arrayZ[0].fromCoord)
      return inputvalue - arrayZ[0].fromCoord + arrayZ[0].toCoord;

    unsigned int i;
    unsigned int count = len - 1;
    for (i = 1; i < count && inputvalue > arrayZ[i].fromCoord; i++)
      ;

    if (inputvalue >= arrayZ[i].fromCoord)
      return inputvalue - arrayZ[i].fromCoord + arrayZ[i].toCoord;

    if (unlikely (arrayZ[i-1].fromCoord == arrayZ[i].fromCoord))
      return arrayZ[i-1].toCoord;

    int denom = arrayZ[i].fromCoord - arrayZ[i-1].fromCoord;
    return arrayZ[i-1].toCoord +
	   ((arrayZ[i].toCoord - arrayZ[i-1].toCoord) *
	    (inputvalue - arrayZ[i-1].fromCoord) + denom/2) / denom;
#undef toCoord
#undef fromCoord
  }