if( cc == 0 )
			{
				TIFFWarningExt(tif->tif_clientdata, module,
					       "Terminating PackBitsDecode due to lack of data.");
				break;
			}
			occ -= n;
			b = *bp++;
			cc--;
			while (n-- > 0)
				*op++ = (uint8) b;