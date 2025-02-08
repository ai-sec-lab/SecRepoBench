void GrShape::simplifyRect(const SkRect& rect, SkPathDirection dir, unsigned start,
                           unsigned flags) {
    if (!rect.width() || !rect.height()) {
        if (flags & kSimpleFill_Flag) // <MASK>
    } else {
        if (!this->isRect()) {
            this->setType(Type::kRect);
            fRect = rect;
            this->setPathWindingParams(dir, start);
        } else {
            // If starting as a rect, the provided rect/winding params should already be set
            SkASSERT(fRect == rect && this->dir() == dir && this->startIndex() == start);
        }
        if (flags & kMakeCanonical_Flag) {
            fRect.sort();
        }
    }
}