SkPathRef::Editor::Editor(sk_sp<SkPathRef>* pathRef,
                          int incReserveVerbs,
                          int extraReservePoints)
{
    if ((*pathRef)->unique()) {
        (*pathRef)->incReserve(incReserveVerbs, extraReservePoints);
    } else {
        SkPathRef* copy = new SkPathRef;
        copy->copy(**pathRef, incReserveVerbs, extraReservePoints);
        pathRef->reset(copy);
    }
    fPathRef = pathRef->get();
    fPathRef->callGenIDChangeListeners();
    fPathRef->fGenerationID = 0;
    fPathRef->fBoundsIsDirty = true;
    SkDEBUGCODE(sk_atomic_inc(&fPathRef->fEditorsAttached);)
}