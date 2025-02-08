Status InvSqueeze(Image &input, std::vector<SqueezeParams> parameters,
                  ThreadPool *pool) {
  if (parameters.empty()) {
    DefaultSqueezeParameters(&parameters, input);
  }

  for (int i = parameters.size() - 1; i >= 0; i--) {
    JXL_RETURN_IF_ERROR(
        CheckMetaSqueezeParams(parameters[i], input.channel.size()));
    bool horizontal = parameters[i].horizontal;
    bool in_place = parameters[i].in_place;
    uint32_t beginc = parameters[i].begin_c;
    uint32_t endc = parameters[i].begin_c + parameters[i].num_c - 1;
    uint32_t offset;
    if (in_place) {
      offset = endc + 1;
    } else {
      offset = input.channel.size() + beginc - endc - 1;
    }
    if (beginc < input.nb_meta_channels) {
      // This is checked in MetaSqueeze.
      JXL_ASSERT(input.nb_meta_channels > parameters[i].num_c);
      input.nb_meta_channels -= parameters[i].num_c;
    }

    for (uint32_t c = beginc; c <= endc; c++) {
      uint32_t rc = offset + c - beginc;
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
    input.channel.erase(input.channel.begin() + offset,
                        input.channel.begin() + offset + (endc - beginc + 1));
  }
  return true;
}