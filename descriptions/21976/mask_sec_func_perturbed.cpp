void GrShape::simplifyRect(const SkRect& rect, SkPathDirection direction, unsigned start,
                           unsigned flags) {
    if (!rect.width() || !rect.height()) {
        if (flags & kSimpleFill_Flag) // <MASK>
    } else {
        if (!this->isRect()) {
            this->setType(Type::kRect);
            fRect = rect;
            this->setPathWindingParams(direction, start);
        } else {
            // If starting as a rect, the provided rect/winding params should already be set
            SkASSERT(fRect == rect && this->dir() == direction && this->startIndex() == start);
        }
        if (flags & kMakeCanonical_Flag) {
            fRect.sort();
        }
    }
}