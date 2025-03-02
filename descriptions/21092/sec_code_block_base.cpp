if (value <= arrayZ[0].fromCoord)
      return value - arrayZ[0].fromCoord + arrayZ[0].toCoord;

    unsigned int i;
    unsigned int count = len - 1;
    for (i = 1; i < count && value > arrayZ[i].fromCoord; i++)
      ;

    if (value >= arrayZ[i].fromCoord)
      return value - arrayZ[i].fromCoord + arrayZ[i].toCoord;

    if (unlikely (arrayZ[i-1].fromCoord == arrayZ[i].fromCoord))
      return arrayZ[i-1].toCoord;

    int denom = arrayZ[i].fromCoord - arrayZ[i-1].fromCoord;