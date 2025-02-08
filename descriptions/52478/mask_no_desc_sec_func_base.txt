void AnimResolver::UpdateAnimRangeSetup() {
    // XXX doesn't work yet (hangs if more than one envelope channels needs to be interpolated)

    for (std::list<LWO::Envelope>::iterator it = envelopes.begin(); it != envelopes.end(); ++it) {
        if ((*it).keys.empty()) continue;

        const double my_first = (*it).keys.front().time;
        const double my_last = (*it).keys.back().time;

        const double delta = my_last - my_first;
        // <MASK>

        // process post behavior
        switch ((*it).post) {

        case LWO::PrePostBehaviour_OffsetRepeat:
        case LWO::PrePostBehaviour_Repeat:
        case LWO::PrePostBehaviour_Oscillate:

            break;

        default:
            // silence compiler warning
            break;
        }
    }
}