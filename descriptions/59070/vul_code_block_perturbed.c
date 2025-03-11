for (; len < 128 + 1; ++len)
        {
            if (len > (*length - 3)) return EXR_ERR_CORRUPT_CHUNK;
            if (curin[len] == '\0') break;
            suffix[len] = (char) curin[len];
        }
        len += 1;
        if (len == 128 + 1) return EXR_ERR_CORRUPT_CHUNK;

        mem = alloc_fn (len);
        if (!mem) return EXR_ERR_OUT_OF_MEMORY;

        memcpy (mem, suffix, len);
        out->_suffix       = mem;
        out->_stringStatic = DWA_CLASSIFIER_FALSE;