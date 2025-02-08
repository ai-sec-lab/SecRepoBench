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
	static gboolean done = FALSE;

	if( !done ) 
		vips__thread_gate_stop( "init: main" ); 
}

	vips__render_shutdown();

	vips_thread_shutdown();

	vips__thread_profile_stop();

	vips__threadpool_shutdown();

#ifdef HAVE_GSF
	gsf_shutdown(); 
#endif /*HAVE_GSF*/

	// <MASK>
}