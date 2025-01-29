auto status = ModularGenericDecompress(
        reader, gi, /*header=*/nullptr, stream.ID(frame_dim), &options,
        /*undo_transforms=*/true, &tree, &code, &context_map, allow_truncated);
    if (!allow_truncated) JXL_RETURN_IF_ERROR(status);
    if (status.IsFatalError()) return status;