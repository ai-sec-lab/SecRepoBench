if( pdf->pages[i].width < 1 ||
			pdf->pages[i].height < 1 ||
			pdf->pages[i].width > VIPS_MAX_COORD ||
			pdf->pages[i].height > VIPS_MAX_COORD ) {
			vips_error( class->nickname, 
				"%s", _( "page size out of range" ) );
			return( -1 ); 
		}

		if( pdf->pages[i].width > pdf->image.width )
			pdf->image.width = pdf->pages[i].width;
		pdf->image.height += pdf->pages[i].height;

		top += pdf->pages[i].height;