if (dict->subdict != NULL) {
        xmlDictEntry *subEntry;
        unsigned subHashValue;

        if (pref == NULL)
            subHashValue = xmlDictHashName(dict->subdict->seed, name, len,
                                           &len);
        else
            subHashValue = xmlDictHashQName(dict->subdict->seed, pref, name,
                                            &plen, &len);
        subEntry = xmlDictFindEntry(dict->subdict, pref, name, klen,
                                    subHashValue, &found);
        if (found)
            return(subEntry);
    }