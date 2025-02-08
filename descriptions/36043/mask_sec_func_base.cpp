Status MetaSqueeze(Image &image, std::vector<SqueezeParams> *parameters) {
  if (parameters->empty()) {
    DefaultSqueezeParameters(parameters, image);
  }

  for (size_t i = 0; i < parameters->size(); i++) {
    JXL_RETURN_IF_ERROR(
        CheckMetaSqueezeParams((*parameters)[i], image.channel.size()));
    bool horizontal = (*parameters)[i].horizontal;
    bool in_place = (*parameters)[i].in_place;
    uint32_t beginc = (*parameters)[i].begin_c;
    uint32_t endc = (*parameters)[i].begin_c + (*parameters)[i].num_c - 1;

    uint32_t offset;
    // <MASK>
  }
  return true;
}