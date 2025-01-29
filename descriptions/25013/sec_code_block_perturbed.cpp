for (unsigned cppindex = 0; cppindex < inner_maps.length; cppindex++)
      inner_maps[cppindex].fini ();

    return_trace (
        !c->serializer->in_error()
        && varstore_prime->dataSets);