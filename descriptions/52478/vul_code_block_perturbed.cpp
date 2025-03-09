const size_t old_size = (*it).keys.size();

        const float value_delta = (*it).keys.back().value - (*it).keys.front().value;

        // NOTE: We won't handle reset, linear and constant here.
        // See DoInterpolation() for their implementation.

        // process pre behavior
        switch ((*it).pre) {
        case LWO::PrePostBehaviour_OffsetRepeat:
        case LWO::PrePostBehaviour_Repeat:
        case LWO::PrePostBehaviour_Oscillate: {
            const double start_time = delta - std::fmod(firsttime - first, delta);
            std::vector<LWO::Key>::iterator n = std::find_if((*it).keys.begin(), (*it).keys.end(),
                                                    [start_time](double t) { return start_time > t; }),
                                            m;

            size_t ofs = 0;
            if (n != (*it).keys.end()) {
                // copy from here - don't use iterators, insert() would invalidate them
                ofs = (*it).keys.end() - n;
                (*it).keys.insert((*it).keys.begin(), ofs, LWO::Key());

                std::copy((*it).keys.end() - ofs, (*it).keys.end(), (*it).keys.begin());
            }

            // do full copies. again, no iterators
            const unsigned int num = (unsigned int)((firsttime - first) / delta);
            (*it).keys.resize((*it).keys.size() + num * old_size);

            n = (*it).keys.begin() + ofs;
            bool reverse = false;
            for (unsigned int i = 0; i < num; ++i) {
                m = n + old_size * (i + 1);
                std::copy(n, n + old_size, m);
                const bool res = ((*it).pre == LWO::PrePostBehaviour_Oscillate);
                reverse = !reverse;
                if (res && reverse) {
                    std::reverse(m, m + old_size - 1);
                }
            }

            // update time values
            n = (*it).keys.end() - (old_size + 1);
            double cur_minus = delta;
            unsigned int tt = 1;
            for (const double tmp = delta * (num + 1); cur_minus <= tmp; cur_minus += delta, ++tt) {
                m = (delta == tmp ? (*it).keys.begin() : n - (old_size + 1));
                for (; m != n; --n) {
                    (*n).time -= cur_minus;

                    // offset repeat? add delta offset to key value
                    if ((*it).pre == LWO::PrePostBehaviour_OffsetRepeat) {
                        (*n).value += tt * value_delta;
                    }
                }
            }
            break;
        }
        default:
            // silence compiler warning
            break;
        }