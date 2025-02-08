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
    // <MASK>
  }
  return true;
}