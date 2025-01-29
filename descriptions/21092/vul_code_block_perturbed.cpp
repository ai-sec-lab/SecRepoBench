if (inputvalue <= arrayZ[0].fromCoord)
      return inputvalue - arrayZ[0].fromCoord + arrayZ[0].toCoord;

    unsigned int i;
    unsigned int count = len;
    for (i = 1; i < count && inputvalue > arrayZ[i].fromCoord; i++)
      ;

    if (inputvalue >= arrayZ[i].fromCoord)
      return inputvalue - arrayZ[i].fromCoord + arrayZ[i].toCoord;