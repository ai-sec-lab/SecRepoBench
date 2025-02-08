if (context->plan->drop_hints)
    {
      // AnchorFormat 2 and 3 just containing extra hinting information, so
      // if hints are being dropped convert to format 1.
      if (u.format != 1 && u.format != 2 && u.format != 3)
        return_trace (false);
      return_trace (bool (reinterpret_cast<Anchor *> (u.format1.copy (context->serializer))));
    }