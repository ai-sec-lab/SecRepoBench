{
					if (j+1 >= GF_ARRAY_LENGTH(szLineConv))
						break;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
					szLineConv[j] = linebuffer[i];
					i++;
					j++;
				}
				/*UTF8 4 bytes char*/
				else if ( (szLine[i] & 0xf8) == 0xf0) {
					if (j+2 >= GF_ARRAY_LENGTH(szLineConv))
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