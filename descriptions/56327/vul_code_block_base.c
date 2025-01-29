#if 0 /* in case time.h with clock() isn't available for some reason */
	e->stats_frames_interval = 0;
	e->old_frames_written = 0;
#else
	e->old_clock_t = 0;
#endif

	memset(&e->info, 0, sizeof(e->info));

	e->format = options.format;