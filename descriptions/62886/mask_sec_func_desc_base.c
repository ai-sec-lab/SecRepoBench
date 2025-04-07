ATTRIBUTE_NO_SANITIZE_INTEGER
static const xmlDictEntry *
xmlDictLookupInternal(xmlDictPtr dict, const xmlChar *prefix,
                      const xmlChar *name, int maybeLen, int update) {
    xmlDictEntry *entry = NULL;
    const xmlChar *ret;
    unsigned hashValue;
    size_t maxLen, len, plen, klen;
    int found = 0;

    if ((dict == NULL) || (name == NULL))
	return(NULL);

    maxLen = (maybeLen < 0) ? SIZE_MAX : (size_t) maybeLen;

    if (prefix == NULL) {
        hashValue = xmlDictHashName(dict->seed, name, maxLen, &len);
        if (len > INT_MAX)
            return(NULL);
        klen = len;
    } else {
        hashValue = xmlDictHashQName(dict->seed, prefix, name, &plen, &len);
        if ((len > INT_MAX) || (plen >= INT_MAX - len))
            return(NULL);
        klen = plen + 1 + len;
    }

    if ((dict->limit > 0) && (klen >= dict->limit))
        return(NULL);

    /*
     * Check for an existing entry
     */
    if (dict->size > 0)
        entry = xmlDictFindEntry(dict, prefix, name, klen, hashValue, &found);
    if (found)
        return(entry);

    // Check if the main dictionary has a subdictionary.
    // If a subdictionary exists, compute the hash value for the provided name,
    // using either the name alone or the combination of prefix and name,
    // depending on whether a prefix is given.
    // Attempt to find an existing entry in the subdictionary using the computed hash value.
    // If an entry is found in the subdictionary, return that entry.
    // <MASK>

    if (!update)
        return(NULL);

    /*
     * Grow the hash table if needed
     */
    if (dict->nbElems + 1 > dict->size / MAX_FILL_DENOM * MAX_FILL_NUM) {
        unsigned newSize, mask, displ, pos;

        if (dict->size == 0) {
            newSize = MIN_HASH_SIZE;
        } else {
            if (dict->size >= MAX_HASH_SIZE)
                return(NULL);
            newSize = dict->size * 2;
        }
        if (xmlDictGrow(dict, newSize) != 0)
            return(NULL);

        /*
         * Find new entry
         */
        mask = dict->size - 1;
        displ = 0;
        pos = hashValue & mask;
        entry = &dict->table[pos];

        while ((entry->hashValue != 0) &&
               ((pos - entry->hashValue) & mask) >= displ) {
            displ++;
            pos++;
            entry++;
            if ((pos & mask) == 0)
                entry = dict->table;
        }
    }

    if (prefix == NULL)
        ret = xmlDictAddString(dict, name, len);
    else
        ret = xmlDictAddQString(dict, prefix, plen, name, len);
    if (ret == NULL)
        return(NULL);

    /*
     * Shift the remainder of the probe sequence to the right
     */
    if (entry->hashValue != 0) {
        const xmlDictEntry *end = &dict->table[dict->size];
        const xmlDictEntry *cur = entry;

        do {
            cur++;
            if (cur >= end)
                cur = dict->table;
        } while (cur->hashValue != 0);

        if (cur < entry) {
            /*
             * If we traversed the end of the buffer, handle the part
             * at the start of the buffer.
             */
            memmove(&dict->table[1], dict->table,
                    (char *) cur - (char *) dict->table);
            cur = end - 1;
            dict->table[0] = *cur;
        }

        memmove(&entry[1], entry, (char *) cur - (char *) entry);
    }

    /*
     * Populate entry
     */
    entry->hashValue = hashValue;
    entry->name = ret;

    dict->nbElems++;

    return(entry);
}