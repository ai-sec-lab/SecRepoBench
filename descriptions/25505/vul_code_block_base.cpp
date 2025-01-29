V2i tileMax = tileMin + V2i (tileDesc.xSize - 1, tileDesc.ySize - 1);

    V2i levelMax = dataWindowForLevel
		       (tileDesc, minX, maxX, minY, maxY, lx, ly).max;

    tileMax = V2i (std::min (tileMax[0], levelMax[0]),
		   std::min (tileMax[1], levelMax[1]));