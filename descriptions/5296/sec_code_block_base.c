    std::vector<value_type> curve(65536);

    for (int i = 0; i < num_segments; i++) {
      const Segment& s = segments[i];

      for (int x = xCp[i]; x <= xCp[i + 1]; x++) {
        double diff = x - xCp[i];
        double diff_2 = diff * diff;
        double diff_3 = diff * diff * diff;

        double interpolated = s.a + s.b * diff + s.c * diff_2 + s.d * diff_3;

        if (!std::is_floating_point<value_type>::value) {
          interpolated = std::max(
              interpolated, double(std::numeric_limits<value_type>::min()));
          interpolated = std::min(
              interpolated, double(std::numeric_limits<value_type>::max()));
        }

        curve[x] = interpolated;
      }
    }

    return curve;