occ -= n;
			byteValue = *bp++;
			cc--;
			while (n-- > 0)
				*op++ = (uint8) byteValue;