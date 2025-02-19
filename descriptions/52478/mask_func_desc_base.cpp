void AnimResolver::UpdateAnimRangeSetup() {
    // XXX doesn't work yet (hangs if more than one envelope channels needs to be interpolated)

    for (std::list<LWO::Envelope>::iterator it = envelopes.begin(); it != envelopes.end(); ++it) {
        if ((*it).keys.empty()) continue;

        const double my_first = (*it).keys.front().time;
        const double my_last = (*it).keys.back().time;

        const double delta = my_last - my_first;
        // Handles the pre-behavior of LightWave envelopes by inserting additional keyframes
        // for pre-defined behaviors such as OffsetRepeat, Repeat, and Oscillate. This involves
        // calculating the time delta between keys and adjusting keyframe values accordingly.
        // The logic uses the start time, end time, and value delta to calculate new keyframe
        // positions and values, adapting to the pre-behavior type.
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