/* In dev releases, always show leaks. But not more than once, it's
	 * annoying.
	 */
#ifndef DEBUG_LEAK
	if( vips__leak ) 
#endif /*DEBUG_LEAK*/
	{
		static gboolean completed = FALSE;

		if( !completed &&
			vips_leak() ) 
			exit( 1 );

		completed = TRUE;
	}

	VIPS_FREE( vips__argv0 );
	VIPS_FREE( vips__prgname );
	VIPS_FREEF( vips_g_mutex_free, vips__global_lock );
	VIPS_FREEF( g_timer_destroy, vips__global_timer );