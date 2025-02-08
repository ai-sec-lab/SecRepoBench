SkPathRef::Editor::Editor(sk_sp<SkPathRef>* pathRef,
                          int incReserveVerbs,
                          int incReservePoints)
{
    if ((*pathRef)->unique()) {
        (*pathRef)->incReserve(incReserveVerbs, incReservePoints);
    } else {
        SkPathRef* copy = new SkPathRef;
        copy->copy(**pathRef, incReserveVerbs, incReservePoints);
        pathRef->reset(copy);
    }
    // <MASK>
}