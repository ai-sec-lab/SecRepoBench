if (!bounds.setBoundsCheck(polygonVertices, polygonSize)) {
        return false;
    }
    // get winding direction
    // TODO: we do this for all the polygon routines -- might be better to have the client
    // compute it and pass it in
    int winding = SkGetPolygonWinding(polygonVertices, polygonSize);
    if (0 == winding) {
        return false;
    }

    // Set up vertices
    SkAutoSTMalloc<64, TriangulationVertex> triangulationVertices(polygonSize);
    int prevIndex = polygonSize - 1;
    SkVector v0 = polygonVertices[0] - polygonVertices[prevIndex];
    for (int currIndex = 0; currIndex < polygonSize; ++currIndex) {
        int nextIndex = (currIndex + 1) % polygonSize;

        SkDEBUGCODE(memset(&triangulationVertices[currIndex], 0, sizeof(TriangulationVertex)));
        triangulationVertices[currIndex].fPosition = polygonVertices[currIndex];
        triangulationVertices[currIndex].fIndex = currIndex;
        triangulationVertices[currIndex].fPrevIndex = prevIndex;
        triangulationVertices[currIndex].fNextIndex = nextIndex;
        SkVector v1 = polygonVertices[nextIndex] - polygonVertices[currIndex];
        if (winding*v0.cross(v1) > SK_ScalarNearlyZero*SK_ScalarNearlyZero) {
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kConvex;
        } else {
            triangulationVertices[currIndex].fVertexType = TriangulationVertex::VertexType::kReflex;
        }

        prevIndex = currIndex;
        v0 = v1;
    }

    // Classify initial vertices into a list of convex vertices and a hash of reflex vertices
    // TODO: possibly sort the convexList in some way to get better triangles
    SkTInternalLList<TriangulationVertex> convexList;
    ReflexHash reflexHash(bounds, polygonSize);
    prevIndex = polygonSize - 1;
    for (int currIndex = 0; currIndex < polygonSize; prevIndex = currIndex, ++currIndex) {
        TriangulationVertex::VertexType currType = triangulationVertices[currIndex].fVertexType;
        if (TriangulationVertex::VertexType::kConvex == currType) {
            int nextIndex = (currIndex + 1) % polygonSize;
            TriangulationVertex::VertexType prevType = triangulationVertices[prevIndex].fVertexType;
            TriangulationVertex::VertexType nextType = triangulationVertices[nextIndex].fVertexType;
            // We prioritize clipping vertices with neighboring reflex vertices.
            // The intent here is that it will cull reflex vertices more quickly.
            if (TriangulationVertex::VertexType::kReflex == prevType ||
                TriangulationVertex::VertexType::kReflex == nextType) {
                convexList.addToHead(&triangulationVertices[currIndex]);
            } else {
                convexList.addToTail(&triangulationVertices[currIndex]);
            }
        } else {
            // We treat near collinear vertices as reflex
            reflexHash.add(&triangulationVertices[currIndex]);
        }
    }

    // The general concept: We are trying to find three neighboring vertices where
    // no other vertex lies inside the triangle (an "ear"). If we find one, we clip
    // that ear off, and then repeat on the new polygon. Once we get down to three vertices
    // we have triangulated the entire polygon.
    // In the worst case this is an n^2 algorithm. We can cut down the search space somewhat by
    // noting that only convex vertices can be potential ears, and we only need to check whether
    // any reflex vertices lie inside the ear.
    triangleIndices->setReserve(triangleIndices->count() + 3 * (polygonSize - 2));
    int vertexCount = polygonSize;
    while (vertexCount > 3) {
        bool success = false;
        TriangulationVertex* earVertex = nullptr;
        TriangulationVertex* p0 = nullptr;
        TriangulationVertex* p2 = nullptr;
        // find a convex vertex to clip
        for (SkTInternalLList<TriangulationVertex>::Iter convexIter = convexList.begin();
             convexIter != convexList.end(); ++convexIter) {
            earVertex = *convexIter;
            SkASSERT(TriangulationVertex::VertexType::kReflex != earVertex->fVertexType);

            p0 = &triangulationVertices[earVertex->fPrevIndex];
            p2 = &triangulationVertices[earVertex->fNextIndex];

            // see if any reflex vertices are inside the ear
            bool failed = reflexHash.checkTriangle(p0->fPosition, earVertex->fPosition,
                                                   p2->fPosition, p0->fIndex, p2->fIndex);
            if (failed) {
                continue;
            }

            // found one we can clip
            success = true;
            break;
        }
        // If we can't find any ears to clip, this probably isn't a simple polygon
        if (!success) {
            return false;
        }

        // add indices
        auto indices = triangleIndices->append(3);
        indices[0] = indexMap[p0->fIndex];
        indices[1] = indexMap[earVertex->fIndex];
        indices[2] = indexMap[p2->fIndex];

        // clip the ear
        convexList.remove(earVertex);
        --vertexCount;

        // reclassify reflex verts
        p0->fNextIndex = earVertex->fNextIndex;
        reclassify_vertex(p0, polygonVertices, winding, &reflexHash, &convexList);

        p2->fPrevIndex = earVertex->fPrevIndex;
        reclassify_vertex(p2, polygonVertices, winding, &reflexHash, &convexList);
    }

    // output indices
    for (SkTInternalLList<TriangulationVertex>::Iter vertexIter = convexList.begin();
         vertexIter != convexList.end(); ++vertexIter) {
        TriangulationVertex* vertex = *vertexIter;
        *triangleIndices->push() = indexMap[vertex->fIndex];
    }

    return true;