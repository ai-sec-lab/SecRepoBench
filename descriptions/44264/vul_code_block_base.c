{
                    while (pdfpsbuf < buflim && *pdfpsbuf != char_EOL && *pdfpsbuf != '\f' &&
                           *pdfpsbuf != char_CR)
                        pdfpsbuf++;

                    if (*pdfpsbuf == char_EOL)
                        pdfpsbuf++;
                }
                break;