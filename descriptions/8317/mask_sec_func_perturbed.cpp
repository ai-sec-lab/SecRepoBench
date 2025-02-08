Router *RouterTable::Allocate(uint8_t routerId)
{
    // <MASK>

    rval = GetRouter(routerId);
    rval->SetLastHeard(TimerMilli::GetNow());

    mRouterIdSequence++;
    mRouterIdSequenceLastUpdated = TimerMilli::GetNow();
    GetNetif().GetMle().ResetAdvertiseInterval();

    otLogInfoMle(GetInstance(), "Allocate router id %d", routerId);

exit:
    return rval;
}