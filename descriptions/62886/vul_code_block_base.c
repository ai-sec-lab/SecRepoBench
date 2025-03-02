if (dict->subdict != NULL) {
        xmlDictEntry *subEntry;
        unsigned subHashValue;

        if (prefix == NULL)
            subHashValue = xmlDictHashName(dict->subdict->seed, name, len,
                                           &len);
        else
            subHashValue = xmlDictHashQName(dict->subdict->seed, prefix, name,
                                            &plen, &len);
        subEntry = xmlDictFindEntry(dict->subdict, prefix, name, klen,
                                    subHashValue, &found);
        if (found)
            return(subEntry);
    }