for (unsigned i = 0; i < inner_maps.length; i++)
      inner_maps[i].fini ();

    return_trace (
        !c->serializer->in_error()
        && varstore_prime->dataSets);