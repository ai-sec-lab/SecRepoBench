void
vips_shutdown( void )
{
#ifdef DEBUG
	printf( "vips_shutdown:\n" );
#endif /*DEBUG*/

	vips_cache_drop_all();

#if ENABLE_DEPRECATED
	im_close_plugins();
#endif

	/* Mustn't run this more than once. Don't use the VIPS_GATE macro,
	 * since we don't for gate start.
	 */
{
	static gboolean completed = FALSE;

	if( !completed ) 
		vips__thread_gate_stop( "init: main" ); 
}

	vips__render_shutdown();

	vips_thread_shutdown();

	vips__thread_profile_stop();

	vips__threadpool_shutdown();

#ifdef HAVE_GSF
	gsf_shutdown(); 
#endif /*HAVE_GSF*/

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
}