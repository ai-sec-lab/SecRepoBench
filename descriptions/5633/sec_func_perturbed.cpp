SkPathRef* SkPathRef::CreateFromBuffer(SkRBuffer* buffer) {
    std::unique_ptr<SkPathRef> ref(new SkPathRef);

    int32_t packed;
    if (!buffer->readS32(&packed)) {
        return nullptr;
    }

    ref->fIsFinite = (packed >> kIsFinite_SerializationShift) & 1;
    uint8_t segMask = (packed >> kSegmentMask_SerializationShift) & 0xF;

    int32_t verbCount, pointCount, conicCount;
    if (!buffer->readU32(&(ref->fGenerationID)) ||
        !buffer->readS32(&verbCount)            || (verbCount  < 0) ||
        !buffer->readS32(&pointCount)           || (pointCount < 0) ||
        !buffer->readS32(&conicCount)           || (conicCount < 0))
    {
        return nullptr;
    }

    uint64_t pointSize64 = sk_64_mul(pointCount, sizeof(SkPoint));
    uint64_t conicSize64 = sk_64_mul(conicCount, sizeof(SkScalar));
    if (!SkTFitsIn<size_t>(pointSize64) || !SkTFitsIn<size_t>(conicSize64)) {
        return nullptr;
    }

    size_t verbSize = verbCount * sizeof(uint8_t);
    size_t pointSize = SkToSizeT(pointSize64);
    size_t conicSize = SkToSizeT(conicSize64);

    {
        uint64_t requiredBufferSize = sizeof(SkRect);
        requiredBufferSize += verbSize;
        requiredBufferSize += pointSize;
        requiredBufferSize += conicSize;
        if (buffer->available() < requiredBufferSize) {
            return nullptr;
        }
    }

    ref->resetToSize(verbCount, pointCount, conicCount);
    SkASSERT(verbCount  == ref->countVerbs());
    SkASSERT(pointCount == ref->countPoints());
    SkASSERT(conicCount == ref->fConicWeights.count());

    if (!buffer->read(ref->verbsMemWritable(), verbSize) ||
        !buffer->read(ref->fPoints, pointSize) ||
        !buffer->read(ref->fConicWeights.begin(), conicSize) ||
        !buffer->read(&ref->fBounds, sizeof(SkRect))) {
        return nullptr;
    }

    // Check that the verbs are valid, and imply the correct number of pts and conics
    {
        int pCount, cCount;
        if (!validate_verb_sequence(ref->verbsMemBegin(), ref->countVerbs())) {
            return nullptr;
        }
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
    }

    ref->fBoundsIsDirty = false;

    // resetToSize clears fSegmentMask and fIsOval
    ref->fSegmentMask = segMask;
    return ref.release();
}