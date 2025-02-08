if( pdf->pages[i].width > pdf->image.width )
			pdf->image.width = pdf->pages[i].width;
		pdf->image.height += pdf->pages[i].height;

		top += pdf->pages[i].height;