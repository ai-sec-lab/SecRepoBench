if (in_place) {
      offset = endc + 1;
    } else {
      offset = image.channel.size();
    }
    for (uint32_t c = beginc; c <= endc; c++) {
      if (image.channel[c].hshift > 30 || image.channel[c].vshift > 30) {
        return JXL_FAILURE("Too many squeezes: shift > 30");
      }
      size_t w = image.channel[c].w;
      size_t h = image.channel[c].h;
      if (horizontal) {
        image.channel[c].w = (w + 1) / 2;
        image.channel[c].hshift++;
        w = w - (w + 1) / 2;
      } else {
        image.channel[c].h = (h + 1) / 2;
        image.channel[c].vshift++;
        h = h - (h + 1) / 2;
      }
      image.channel[c].shrink();
      Channel dummy(w, h);
      dummy.hshift = image.channel[c].hshift;
      dummy.vshift = image.channel[c].vshift;

      image.channel.insert(image.channel.begin() + offset + (c - beginc),
                           std::move(dummy));
    }