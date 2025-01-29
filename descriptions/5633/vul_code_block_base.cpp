if (!deduce_pts_conics(ref->verbsMemBegin(), ref->countVerbs(), &pCount, &cCount) ||
            pCount != ref->countPoints() || cCount != ref->fConicWeights.count()) {
            return nullptr;
        }
        if (!validate_conic_weights(ref->fConicWeights.begin(), ref->fConicWeights.count())) {
            return nullptr;
        }
        // Check that the bounds match the serialized bounds.
        SkRect bounds;
        if (ComputePtBounds(&bounds, *ref) != SkToBool(ref->fIsFinite) || bounds != ref->fBounds) {
            return nullptr;
        }