const ieee1905_fragment_key *key = (const ieee1905_fragment_key *)keypointer;

    if (!key || !key->src.data || !key->dst.data) {
        return 0;
    }

    const guint8 src_len = key->src.len;
    const guint8 dst_len = key->dst.len;
    const guint8 hash_buf_len = src_len + dst_len + sizeof(guint8) + sizeof(guint32);
    guint8* hash_buf = (guint8*)wmem_alloc(wmem_packet_scope(), hash_buf_len);

    memcpy(hash_buf, key->src.data, src_len);
    memcpy(&hash_buf[src_len], key->dst.data, dst_len);
    hash_buf[src_len + dst_len] = key->frag_id;
    memcpy(&hash_buf[src_len + dst_len + sizeof(guint8)], &key->vlan_id, sizeof(guint32));
    hash_val = wmem_strong_hash((const guint8 *)hash_buf, hash_buf_len);