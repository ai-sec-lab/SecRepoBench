SkPathRef::Editor ed(&fPathRef, src.fPathRef->countPoints(), src.fPathRef->countVerbs());

    const SkPoint* pts = src.fPathRef->pointsEnd();
    // we will iterator through src's verbs backwards
    const uint8_t* verbs = src.fPathRef->verbsMemBegin(); // points at the last verb
    const uint8_t* verbsEnd = src.fPathRef->verbs(); // points just past the first verb
    const SkScalar* conicWeights = src.fPathRef->conicWeightsEnd();