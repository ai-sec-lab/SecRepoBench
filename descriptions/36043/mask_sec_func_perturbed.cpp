Status MetaSqueeze(Image &image, std::vector<SqueezeParams> *squeezeparams) {
  if (squeezeparams->empty()) {
    DefaultSqueezeParameters(squeezeparams, image);
  }

  for (size_t i = 0; i < squeezeparams->size(); i++) {
    JXL_RETURN_IF_ERROR(
        CheckMetaSqueezeParams((*squeezeparams)[i], image.channel.size()));
    bool horizontal = (*squeezeparams)[i].horizontal;
    bool in_place = (*squeezeparams)[i].in_place;
    uint32_t beginc = (*squeezeparams)[i].begin_c;
    uint32_t endc = (*squeezeparams)[i].begin_c + (*squeezeparams)[i].num_c - 1;

    uint32_t offset;
    // <MASK>
  }
  return true;
}