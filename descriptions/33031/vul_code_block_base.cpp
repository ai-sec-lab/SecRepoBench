if (c->plan->drop_hints)
      // AnchorFormat 2 and 3 just containing extra hinting information, so
      // if hints are being dropped convert to format 1.
      return_trace (bool (reinterpret_cast<Anchor *> (u.format1.copy (c->serializer))));