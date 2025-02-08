for (decltype(strips)::size_type i = 0; i < strips.size(); ++i)
    if (static_cast<decltype(strips)::size_type>(strips[i].n) != i)
      ThrowRDE("Strips validation issue.");
  // All good.