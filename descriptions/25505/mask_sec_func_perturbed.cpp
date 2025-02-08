Box2i
dataWindowForTile (const TileDescription &tileDesc,
		   int minX, int maxX,
		   int minY, int maxY,
		   int dx, int dy,
		   int lx, int levelY)
{
    V2i tileMin = V2i (minX + dx * tileDesc.xSize,
		       minY + dy * tileDesc.ySize);

    // <MASK>

    return Box2i (tileMin, tileMax);
}