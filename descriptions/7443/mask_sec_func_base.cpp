sk_sp<SkSpecialImage> SkDisplacementMapEffect::onFilterImage(SkSpecialImage* source,
                                                             const Context& ctx,
                                                             SkIPoint* offset) const {
    SkIPoint colorOffset = SkIPoint::Make(0, 0);
    sk_sp<SkSpecialImage> color(this->filterInput(1, source, ctx, &colorOffset));
    if (!color) {
        return nullptr;
    }

    SkIPoint displOffset = SkIPoint::Make(0, 0);
    // Creation of the displacement map should happen in a non-colorspace aware context. This
    // texture is a purely mathematical construct, so we want to just operate on the stored
    // values. Consider:
    // User supplies an sRGB displacement map. If we're rendering to a wider gamut, then we could
    // end up filtering the displacement map into that gamut, which has the effect of reducing
    // the amount of displacement that it represents (as encoded values move away from the
    // primaries).
    // With a more complex DAG attached to this input, it's not clear that working in ANY specific
    // color space makes sense, so we ignore color spaces (and gamma) entirely. This may not be
    // ideal, but it's at least consistent and predictable.
    Context displContext(ctx.ctm(), ctx.clipBounds(), ctx.cache(), OutputProperties(nullptr));
    sk_sp<SkSpecialImage> displ(this->filterInput(0, source, displContext, &displOffset));
    if (!displ) {
        return nullptr;
    }

    const SkIRect srcBounds = SkIRect::MakeXYWH(colorOffset.x(), colorOffset.y(),
                                                color->width(), color->height());

    // Both paths do bounds checking on color pixel access, we don't need to
    // pad the color bitmap to bounds here.
    SkIRect bounds;
    if (!this->applyCropRect(ctx, srcBounds, &bounds)) {
        return nullptr;
    }

    SkIRect displBounds;
    displ = this->applyCropRect(ctx, displ.get(), &displOffset, &displBounds);
    if (!displ) {
        return nullptr;
    }

    if (!bounds.intersect(displBounds)) {
        return nullptr;
    }

    const SkIRect colorBounds = bounds.makeOffset(-colorOffset.x(), -colorOffset.y());
    // If the offset overflowed (saturated) then we have to abort, as we need their
    // dimensions to be equal. See https://bugs.chromium.org/p/oss-fuzz/issues/detail?id=7209
    // <MASK>
}