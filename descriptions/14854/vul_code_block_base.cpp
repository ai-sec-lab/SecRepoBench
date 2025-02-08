mplOffset = offset;
            mplLength = option.GetLength();

            if (mplOffset == sizeof(ip6Header) + sizeof(hbh) && hbh.GetLength() == 0)
            {
                // first and only IPv6 Option, remove IPv6 HBH Option header
                remove = true;
            }
            else if (mplOffset + 8 == endOffset)
            {
                // last IPv6 Option, remove last 8 bytes
                remove = true;
            }