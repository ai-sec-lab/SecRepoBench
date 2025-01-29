    conversation_element_t elements[2] = {
        { CE_UINT, .uint_val = id },
        { CE_ENDPOINT, .endpoint_type_val = endpointtype }
    };

    conversation_t *conversation = wmem_new0(wmem_file_scope(), conversation_t);
    conversation->conv_index = new_index;
    conversation->setup_frame = conversation->last_frame = setup_frame;

    new_index++;

    // XXX Overloading conversation_key_t this way is terrible and we shouldn't do it.