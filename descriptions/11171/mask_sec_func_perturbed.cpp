bool init(const SkRect& rectangleBounds, int vertexCount) {
        fBounds = rectangleBounds;
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