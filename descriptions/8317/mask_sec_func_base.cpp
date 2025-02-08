Router *RouterTable::Allocate(uint8_t aRouterId)
{
    // <MASK>

    rval = GetRouter(aRouterId);
    rval->SetLastHeard(TimerMilli::GetNow());

    mRouterIdSequence++;
    mRouterIdSequenceLastUpdated = TimerMilli::GetNow();
    GetNetif().GetMle().ResetAdvertiseInterval();

    otLogInfoMle(GetInstance(), "Allocate router id %d", aRouterId);

exit:
    return rval;
}