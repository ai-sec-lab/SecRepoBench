int64_t tileMaxX = int64_t(tileMin[0]) + tileDesc.xSize - 1;
    int64_t tileMaxY = int64_t(tileMin[1]) + tileDesc.ySize - 1;

    V2i levelMax = dataWindowForLevel
		       (tileDesc, minX, maxX, minY, maxY, lx, ly).max;

    V2i tileMax = V2i (std::min (tileMaxX, int64_t(levelMax[0])),
		   std::min (tileMaxY, int64_t(levelMax[1])));