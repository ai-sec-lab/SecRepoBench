bool SkTriangulateSimplePolygon(const SkPoint* polygonVerts, uint16_t* indexMap, int polygonSize,
                                SkTDArray<uint16_t>* triangleIndices) {
    if (polygonSize < 3) {
        return false;
    }
    // need to be able to represent all the vertices in the 16-bit indices
    if (polygonSize >= std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    // get bounds
    SkRect bounds;
    // <MASK>
}