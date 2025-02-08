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
    // <MASK>
}