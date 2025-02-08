/* Our file read could conceivably have read back to the point where we read
             * part of the 'startxref' token, but not all of it. So we want to preserve
             * the data in the buffer, but not all of it obviously! The 'startxref' should be followed
             * by a line ending, so above we keep a note of the last line ending. If we found one, then
             * we preserve from the start of the buffer to that point. This could slow us up if the file
             * Is broken, or has a load of junk after the EOF, because we could potentially be saving a
             * lot of data on each pass, but that's only going to happen with bad files.
             * Note we reduce the number of bytes to read so that it just fits into the buffer up to the
             * beginning of the data we preserved.
             */
            if (last_lineend) {
                leftover = last_lineend - Buffer;
                memmove(Buffer + bytes - leftover, last_lineend, leftover);
                bytes -= leftover;
            } else
                leftover = 0;