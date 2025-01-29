if (rc < 0) {
        return NULL;
      }
      *off_cbytes = (int32_t)chunk_cbytes;
      if (*off_cbytes < 0 || *off_cbytes > frame->len) {
        BLOSC_TRACE_ERROR("Cannot read the cbytes outside of frame boundary.");
        return NULL;
      }