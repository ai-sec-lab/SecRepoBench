fPathRef = pathRef->get();
    fPathRef->callGenIDChangeListeners();
    fPathRef->fGenerationID = 0;
    fPathRef->fBoundsIsDirty = true;
    SkDEBUGCODE(sk_atomic_inc(&fPathRef->fEditorsAttached);)