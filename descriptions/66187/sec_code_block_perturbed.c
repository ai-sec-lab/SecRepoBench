{
					if (j+1 >= GF_ARRAY_LENGTH(szLineConv) || i+1 >= len)
						break;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
				}
				/*UTF8 4 bytes char*/
				else if ( (linebuffer[i] & 0xf8) == 0xf0) {
					if (j+2 >= GF_ARRAY_LENGTH(szLineConv) || i+2 >= len)
						break;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
				} else {
					i+=1;
					continue;
				}