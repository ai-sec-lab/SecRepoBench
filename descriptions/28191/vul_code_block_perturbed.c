    guint8 hash_buf[17];
    const ieee1905_fragment_key *key = (const ieee1905_fragment_key *)keypointer;

    if (!key || !key->src.data || !key->dst.data) {
        return 0;
    }

    memcpy(hash_buf, key->src.data, 6);
    memcpy(&hash_buf[6], key->dst.data, 6);
    hash_buf[12] = key->frag_id;
    memcpy(&hash_buf[13], &key->vlan_id, 4);
    hash_val = wmem_strong_hash((const guint8 *)hash_buf, 17);