fPathRef = pathRef->get();
    fPathRef->callGenIDChangeListeners();
    fPathRef->fGenerationID = 0;
    SkDEBUGCODE(sk_atomic_inc(&fPathRef->fEditorsAttached);)