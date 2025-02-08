conversation_t *
conversation_new_by_id(const guint32 setup_frame, const endpoint_type etype, const guint32 id)
{
    // <MASK>
   // XXX Overloading conversation_key_t this way is terrible and we shouldn't do it.
    conversation->key_ptr = (conversation_key_t) elements;
    conversation_insert_into_hashtable(conversation_hashtable_id, conversation);

    return conversation;
}