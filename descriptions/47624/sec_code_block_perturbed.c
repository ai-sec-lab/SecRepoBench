conversation_t *conversation = wmem_new0(wmem_file_scope(), conversation_t);
    conversation->conv_index = new_index;
    conversation->setup_frame = conversation->last_frame = setup_frame;

    new_index++;

    conversation_element_t *elements = wmem_alloc(wmem_file_scope(), sizeof(conversation_element_t) * 2);
    elements[0].type = CE_UINT;
    elements[0].uint_val = id;
    elements[1].type = CE_ENDPOINT;
    elements[1].endpoint_type_val = endpointtype;