void PhaseOneDecompressor::prepareStrips() {
  // The 'strips' vector should contain exactly one element per row of image.

  // If the length is different, then the 'strips' vector is clearly incorrect.
  if (strips.size() != static_cast<decltype(strips)::size_type>(mRaw->dim.y)) {
    ThrowRDE("Height (%u) vs strip count %zu mismatch", mRaw->dim.y,
             strips.size());
  }

  // Now, the strips in 'strips' vector aren't in order.
  // The 'decltype(strips)::value_type::n' is the row number of a strip.
  // We need to make sure that we have every row (0..mRaw->dim.y-1), once.
  // For that, first let's sort them to have monothonically increasting `n`.
  // This will also serialize the per-line outputting.
  std::sort(
      strips.begin(), strips.end(),
      [](const PhaseOneStrip& a, const PhaseOneStrip& b) { return a.n < b.n; });
  // And now ensure that slice number matches the slice's row.
  for (decltype(strips)::size_type i = 0; i < strips.size(); ++i)
    if (static_cast<decltype(strips)::size_type>(strips[i].n) != i)
      ThrowRDE("Strips validation issue.");
  // All good.
}