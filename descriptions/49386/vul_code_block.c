for (i=0; i < TagCount; i++) {

        if (!_cmsReadUInt32Number(io, (cmsUInt32Number *) &Tag.sig)) return FALSE;
        if (!_cmsReadUInt32Number(io, &Tag.offset)) return FALSE;
        if (!_cmsReadUInt32Number(io, &Tag.size)) return FALSE;

        // Perform some sanity check. Offset + size should fall inside file.
        if (Tag.offset + Tag.size > HeaderSize ||
            Tag.offset + Tag.size < Tag.offset)
                  continue;

        Icc -> TagNames[Icc ->TagCount]   = Tag.sig;
        Icc -> TagOffsets[Icc ->TagCount] = Tag.offset;
        Icc -> TagSizes[Icc ->TagCount]   = Tag.size;

       // Search for links
        for (j=0; j < Icc ->TagCount; j++) {

            if ((Icc ->TagOffsets[j] == Tag.offset) &&
                (Icc ->TagSizes[j]   == Tag.size)) {

                Icc ->TagLinked[Icc ->TagCount] = Icc ->TagNames[j];
            }

        }

        Icc ->TagCount++;
    }

    return TRUE;