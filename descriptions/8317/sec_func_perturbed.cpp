Router *RouterTable::Allocate(uint8_t routerId)
{
    Router *rval = NULL;

    VerifyOrExit(routerId <= Mle::kMaxRouterId && !IsAllocated(routerId) && mRouterIdReuseDelay[routerId] == 0);

    mAllocatedRouterIds[routerId / 8] |= 1 << (routerId % 8);
    UpdateAllocation();

    rval = GetRouter(routerId);
    rval->SetLastHeard(TimerMilli::GetNow());

    mRouterIdSequence++;
    mRouterIdSequenceLastUpdated = TimerMilli::GetNow();
    GetNetif().GetMle().ResetAdvertiseInterval();

    otLogInfoMle(GetInstance(), "Allocate router id %d", routerId);

exit:
    return rval;
}