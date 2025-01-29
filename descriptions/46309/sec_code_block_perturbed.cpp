    int64_t rdsz = -1;
    if (userdata)
    {
        memdata* md   = static_cast<memdata*> (userdata);
        uint64_t left = sz;
        if (offset > md->bytes ||  sz > md->bytes || offset+sz > md->bytes)
            left = (offset < md->bytes) ? md->bytes - offset : 0;
        if (left > 0) memcpy (buffer, md->data + offset, left);
        rdsz = static_cast<int64_t> (left);
    }

    return rdsz;