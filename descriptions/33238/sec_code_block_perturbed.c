        if(in.empty()) {
            return;
        }

        // Remove a very common issue when we're parsing file names: spaces at the
        // beginning of the path.
        char last = 0;
        std::string::iterator it = in.begin();
        while (IsSpaceOrNewLine( *it ))++it;
        if (it != in.begin()) {
            in.erase(in.begin(),it+1);
        }

        const char separator = getOsSeparator();
        for (it = in.begin(); it != in.end(); ++it) {
            int remaining = std::distance(in.end(), it);
            // Exclude :// and \\, which remain untouched.
            // https://sourceforge.net/tracker/?func=detail&aid=3031725&group_id=226462&atid=1067632
            if (remaining >= 3 && !strncmp(&*it, "://", 3 )) {
                it += 3;
                continue;
            }
            if (it == in.begin() && remaining >= 2 && !strncmp(&*it, "\\\\", 2)) {
                it += 2;
                continue;
            }

            // Cleanup path delimiters
            if (*it == '/' || (*it) == '\\') {
                *it = separator;

                // And we're removing double delimiters, frequent issue with
                // incorrectly composited paths ...
                if (last == *it) {
                    it = in.erase(it);
                    --it;
                }
            } else if (*it == '%' && in.end() - it > 2) {
                // Hex sequence in URIs
                if( IsHex((&*it)[0]) && IsHex((&*it)[1]) ) {
                    *it = HexOctetToDecimal(&*it);
                    it = in.erase(it+1,it+2);
                    --it;
                }
            }

            last = *it;
        }