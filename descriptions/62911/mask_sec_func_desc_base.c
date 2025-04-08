ATTRIBUTE_NO_SANITIZE_INTEGER
static xmlDictEntry *
xmlDictFindEntry(const xmlDict *dict, const xmlChar *prefix,
                 const xmlChar *name, int len, unsigned hashValue,
                 int *pfound) {
    xmlDictEntry *entry;
    unsigned mask, pos, displ;
    int found = 0;

    mask = dict->size - 1;
    pos = hashValue & mask;
    entry = &dict->table[pos];

    if (entry->hashValue != 0) {
        /*
         * Robin hood hashing: abort if the displacement of the entry
         * is smaller than the displacement of the key we look for.
         * This also stops at the correct position when inserting.
         */
        displ = 0;

        do {
            if (entry->hashValue == hashValue) {
                if 
                // Compare the provided name with the current dictionary entry's name.
                // If the prefix is NULL, compare the names up to the specified length
                // and ensure the entry's name is null-terminated at that position.
                // If a prefix is provided, use a specialized function to compare the qualified names.
                // If a match is found, set the found flag to true and exit the loop.
                // <MASK>
            }

            displ++;
            pos++;
            entry++;
            if ((pos & mask) == 0)
                entry = dict->table;
        } while ((entry->hashValue != 0) &&
                 (((pos - entry->hashValue) & mask) >= displ));
    }

    *pfound = found;
    return(entry);
}