// clang-format off
  for (; (!dat->chain[dat->byte] ||
          dat->chain[dat->byte] == ' ' ||
          dat->chain[dat->byte] == '\t' ||
          dat->chain[dat->byte] == '\r');
       )
    // clang-format on
    {
      dat->byte++;
      if (dat->byte >= dat->size)
        return;
    }