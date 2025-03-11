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