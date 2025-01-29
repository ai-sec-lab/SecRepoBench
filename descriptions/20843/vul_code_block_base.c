				g_free( line );
				vips_error( class->nickname, 
					_( "bad number \"%s\"" ), p );
				return( -1 );