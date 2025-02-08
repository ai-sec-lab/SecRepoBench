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

	static gboolean initializationstarted = FALSE;
	static gboolean done = FALSE;
	const char *vips_min_stack_size;
	// <MASK>

	vips__thread_gate_stop( "init: startup" ); 

	return( 0 );
}