if (!ModularGenericDecompress(reader, gi, /*header=*/nullptr,
                                  stream.ID(frame_dim), &options,
                                  /*undo_transforms=*/true, &tree, &code,
                                  &context_map, allow_truncated) &&
        !allow_truncated) {
      return JXL_FAILURE("Failed to decode modular group");
    }