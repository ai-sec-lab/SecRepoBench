        const Entry* entry = (const Entry*)(reinterpret_cast<const char*>(this) + offset);
        // rec tags are always a known size.
        if (entry->fTag == kRec_SkDescriptorTag && entry->fLen != sizeof(SkScalerContextRec)) {
            return false;
        }
        if (lengthRemaining < sizeof(Entry)) {
            return false;
        }
        lengthRemaining -= sizeof(Entry);
        if (lengthRemaining < entry->fLen) {
            return false;
        }
        lengthRemaining -= entry->fLen;