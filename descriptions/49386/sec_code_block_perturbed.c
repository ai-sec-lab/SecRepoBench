for (i=0; i < TagCount; i++) {

        if (!_cmsReadUInt32Number(io, (cmsUInt32Number *) &TagEntry.sig)) return FALSE;
        if (!_cmsReadUInt32Number(io, &TagEntry.offset)) return FALSE;
        if (!_cmsReadUInt32Number(io, &TagEntry.size)) return FALSE;

        // Perform some sanity check. Offset + size should fall inside file.
        if (TagEntry.size == 0 || TagEntry.offset == 0) continue;
        if (TagEntry.offset + TagEntry.size > HeaderSize ||
            TagEntry.offset + TagEntry.size < TagEntry.offset)
                  continue;

        Icc -> TagNames[Icc ->TagCount]   = TagEntry.sig;
        Icc -> TagOffsets[Icc ->TagCount] = TagEntry.offset;
        Icc -> TagSizes[Icc ->TagCount]   = TagEntry.size;

       // Search for links
        for (j=0; j < Icc ->TagCount; j++) {

            if ((Icc ->TagOffsets[j] == TagEntry.offset) &&
                (Icc ->TagSizes[j]   == TagEntry.size)) {

                Icc ->TagLinked[Icc ->TagCount] = Icc ->TagNames[j];
            }

        }

        Icc ->TagCount++;
    }

    if (Icc->TagCount == 0) {
        cmsSignalError(Icc->ContextID, cmsERROR_RANGE, "Corrupted profile: no tags found");
        return FALSE;
    }
        
     return TRUE;