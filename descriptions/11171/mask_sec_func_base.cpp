bool init(const SkRect& bounds, int vertexCount) {
        fBounds = bounds;
        fNumVerts = 0;
        // <MASK>
        if (!fGridConversion.isFinite()) {
            return false;
        }

        fGrid.setCount(fHCount*fVCount);
        for (int i = 0; i < fGrid.count(); ++i) {
            fGrid[i].reset();
        }

        return true;
    }