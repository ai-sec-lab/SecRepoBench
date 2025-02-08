int
vips_init( const char *argv0 )
{
	extern GType vips_system_get_type( void );
	extern GType write_thread_state_get_type( void );
	extern GType sink_memory_thread_state_get_type( void ); 
	extern GType render_thread_state_get_type( void ); 
	extern GType vips_source_get_type( void ); 
	extern GType vips_source_custom_get_type( void ); 
	extern GType vips_target_get_type( void ); 
	extern GType vips_target_custom_get_type( void ); 
	extern GType vips_g_input_stream_get_type( void ); 

	static gboolean started = FALSE;
	static gboolean done = FALSE;
	const char *vips_min_stack_size;
	gint64 min_stack_size;
	const char *prefix;
	const char *libdir;
#ifdef ENABLE_NLS
	char *locale;
#endif /* ENABLE_NLS */

	/* Two stage done handling: 'done' means we've completed, 'started'
	 * means we're currently initialising. Use this to prevent recursive
	 * invocation.
	 */
	if( done )
		/* Called more than once, we succeeded, just return OK.
		 */
		return( 0 );
	if( started ) 
		/* Recursive invocation, something has broken horribly.
		 * Hopefully the first init will handle it.
		 */
		return( 0 );
	started = TRUE;

	/* Try to set a minimum stacksize, default 2mb. We need to do this
	 * before any threads start.
	 */
	min_stack_size = 2 * 1024 * 1024;
        if( (vips_min_stack_size = g_getenv( "VIPS_MIN_STACK_SIZE" )) )
		min_stack_size = vips__parse_size( vips_min_stack_size );
	(void) set_stacksize( min_stack_size );

	if( g_getenv( "VIPS_INFO" )
#if ENABLE_DEPRECATED
		|| g_getenv( "IM_INFO" )
#endif
	)
		vips_verbose();
	if( g_getenv( "VIPS_PROFILE" ) )
		vips_profile_set( TRUE );
	if( g_getenv( "VIPS_LEAK" ) )
		vips_leak_set( TRUE );
	if( g_getenv( "VIPS_TRACE" ) )
		vips_cache_set_trace( TRUE );
	if( g_getenv( "VIPS_PIPE_READ_LIMIT" ) ) 
		vips_pipe_read_limit = 
			g_ascii_strtoll( g_getenv( "VIPS_PIPE_READ_LIMIT" ),
				NULL, 10 );
	vips_pipe_read_limit_set( vips_pipe_read_limit );

#ifdef G_OS_WIN32
	/* Windows has a limit of 512 files open at once for the fopen() family
	 * of functions, and 2048 for the _open() family. This raises the limit
	 * of fopen() to the same level as _open().
	 *
	 * It will not go any higher than this, unfortunately.  
	 */
	(void) _setmaxstdio( 2048 );
#endif /*G_OS_WIN32*/

	vips__threadpool_init();
	vips__buffer_init();
	vips__meta_init();

	/* This does an unsynchronised static hash table init on first call --
	 * we have to make sure we do this single-threaded. See: 
	 * https://github.com/openslide/openslide/issues/161
	 */
#if !GLIB_CHECK_VERSION( 2, 48, 1 )
	(void) g_get_language_names(); 
#endif

	if( !vips__global_lock )
		vips__global_lock = vips_g_mutex_new();

	if( !vips__global_timer )
		vips__global_timer = g_timer_new();

	VIPS_SETSTR( vips__argv0, argv0 );
	vips__prgname = g_path_get_basename( argv0 );

	vips__thread_profile_attach( "main" );

	/* We can't do VIPS_GATE_START() until command-line processing
	 * happens, since vips__thread_profile may not be set yet. Call
	 * directly. 
	 */
	vips__thread_gate_start( "init: main" ); 
	vips__thread_gate_start( "init: startup" ); 

	/* Try to discover our prefix. 
	 */
        if( (prefix = g_getenv( "VIPSHOME" )) )
		g_info( "VIPSHOME = %s", prefix );
	if( !(prefix = vips_guess_prefix( argv0, "VIPSHOME" )) || 
		!(libdir = vips_guess_libdir( argv0, "VIPSHOME" )) ) 
		return( -1 );

	g_info( "VIPS_PREFIX = %s", VIPS_PREFIX );
	g_info( "VIPS_LIBDIR = %s", VIPS_LIBDIR );
	g_info( "prefix = %s", prefix );
	g_info( "libdir = %s", libdir );

	/* Get i18n .mo files from $VIPSHOME/share/locale/.
	 */
#ifdef ENABLE_NLS
	locale = g_build_filename( prefix, "share", "locale", NULL );
	bindtextdomain( GETTEXT_PACKAGE, locale );
	g_free( locale );
#ifdef HAVE_BIND_TEXTDOMAIN_CODESET
	bind_textdomain_codeset( GETTEXT_PACKAGE, "UTF-8" );
#endif /* HAVE_BIND_TEXTDOMAIN_CODESET */
#endif /* ENABLE_NLS */

	/* Register base vips types.
	 */
	(void) vips_image_get_type();
	(void) vips_region_get_type();
	(void) write_thread_state_get_type();
	(void) sink_memory_thread_state_get_type(); 
	(void) render_thread_state_get_type(); 
	(void) vips_source_get_type(); 
	(void) vips_source_custom_get_type(); 
	(void) vips_target_get_type(); 
	(void) vips_target_custom_get_type(); 
	vips__meta_init_types();
	vips__interpolate_init();

#if ENABLE_DEPRECATED
	im__format_init();
#endif

	/* Start up operator cache.
	 */
	vips__cache_init();

	/* Recomp reordering system.
	 */
	vips__reorder_init();

	/* Start up packages.
	 */
	(void) vips_system_get_type();
	vips_arithmetic_operation_init();
	vips_conversion_operation_init();
	vips_create_operation_init();
	vips_foreign_operation_init();
	vips_resample_operation_init();
	vips_colour_operation_init();
	vips_histogram_operation_init();
	vips_convolution_operation_init();
	vips_freqfilt_operation_init();
	vips_morphology_operation_init();
	vips_draw_operation_init();
	vips_mosaicing_operation_init();
	vips_g_input_stream_get_type(); 

#ifdef ENABLE_MODULES
	/* Load any vips8 modules from the vips libdir. Keep going, even if
	 * some modules fail to load. 
	 *
	 * Only do this if we have been built as a set of loadable
	 * modules, or we might try loading an operation into a library that
	 * already has that operation built in.
	 */
	(void) vips_load_plugins( "%s/vips-modules-%d.%d", 
		libdir, VIPS_MAJOR_VERSION, VIPS_MINOR_VERSION );

#if ENABLE_DEPRECATED
	/* Load any vips8 plugins from the vips libdir.
	 */
	(void) vips_load_plugins( "%s/vips-plugins-%d.%d", 
		libdir, VIPS_MAJOR_VERSION, VIPS_MINOR_VERSION );

	/* Load up any vips7 plugins in the vips libdir. We don't error on 
	 * failure, it's too annoying to have VIPS refuse to start because of 
	 * a broken plugin.
	 */
	if( im_load_plugins( "%s/vips-%d.%d", 
		libdir, VIPS_MAJOR_VERSION, VIPS_MINOR_VERSION ) ) {
		g_warning( "%s", vips_error_buffer() );
		vips_error_clear();
	}

	/* Also load from libdir. This is old and slightly broken behaviour
	 * :-( kept for back compat convenience.
	 */
	if( im_load_plugins( "%s", libdir ) ) {
		g_warning( "%s", vips_error_buffer() );
		vips_error_clear();
	}
#endif /*ENABLE_DEPRECATED*/
#endif /*ENABLE_MODULES*/

	/* Get the run-time compiler going.
	 */
	vips_vector_init();

#ifdef HAVE_GSF
	/* Use this for structured file write.
	 */
	gsf_init();
#endif /*HAVE_GSF*/

#ifdef DEBUG_LEAK
	vips__image_pixels_quark = 
		g_quark_from_static_string( "vips-image-pixels" ); 
#endif /*DEBUG_LEAK*/

	/* If VIPS_WARNING is defined, suppress all warning messages from vips.
	 *
	 * Libraries should not call g_log_set_handler(), it is
	 * supposed to be for the application layer, but this can be awkward to
	 * set up if you are using libvips from something like Ruby. Allow this
	 * env var hack as a workaround. 
	 */
	if( g_getenv( "VIPS_WARNING" )
#if ENABLE_DEPRECATED
		|| g_getenv( "IM_WARNING" )
#endif
	)
		g_log_set_handler( G_LOG_DOMAIN, G_LOG_LEVEL_WARNING, 
			empty_log_handler, NULL );

	/* Block any untrusted operations. This must come after plugin load.
	 */
	if( g_getenv( "VIPS_BLOCK_UNTRUSTED" ) )
		vips_block_untrusted_set( TRUE );

	done = TRUE;

	vips__thread_gate_stop( "init: startup" ); 

	return( 0 );
}