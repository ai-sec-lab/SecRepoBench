pthread_mutex_lock(&global_comp_mutex);

  result = blosc_run_decompression_with_context(
          g_global_context, src, INT32_MAX, dest, destsize);

  pthread_mutex_unlock(&global_comp_mutex);

  return result;