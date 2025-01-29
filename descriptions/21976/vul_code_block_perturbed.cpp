{
            // A zero area, filled shape so go straight to empty
            this->setType(Type::kEmpty);
        } else if (!fRect.width() ^ !fRect.height()) {
            // A line, choose the first point that best matches the starting index
            SkPoint p1 = {rect.fLeft, rect.fTop};
            SkPoint p2 = {rect.fRight, rect.fBottom};
            if (start >= 2 && !(flags & kIgnoreWinding_Flag)) {
                using std::swap;
                swap(p1, p2);
            }
            this->simplifyLine(p1, p2, flags);
        } else {
            // A point (all edges are equal, so start+dir doesn't affect choice)
            this->simplifyPoint({rect.fLeft, rect.fTop}, flags);
        }