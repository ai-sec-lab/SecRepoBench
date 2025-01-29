if (beginc < input.nb_meta_channels) {
      // This is checked in MetaSqueeze.
      JXL_ASSERT(input.nb_meta_channels > parameters[i].num_c);
      input.nb_meta_channels -= parameters[i].num_c;
    }

    for (uint32_t c = beginc; c <= endc; c++) {
      uint32_t rc = displacement + c - beginc;
      if ((input.channel[c].w < input.channel[rc].w) ||
          (input.channel[c].h < input.channel[rc].h)) {
        return JXL_FAILURE("Corrupted squeeze transform");
      }
      if (horizontal) {
        InvHSqueeze(input, c, rc, pool);
      } else {
        InvVSqueeze(input, c, rc, pool);
      }
    }
    input.channel.erase(input.channel.begin() + displacement,
                        input.channel.begin() + displacement + (endc - beginc + 1));