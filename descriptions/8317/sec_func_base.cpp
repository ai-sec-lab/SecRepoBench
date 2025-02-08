Router *RouterTable::Allocate(uint8_t aRouterId)
{
    Router *rval = NULL;

    VerifyOrExit(aRouterId <= Mle::kMaxRouterId && !IsAllocated(aRouterId) && mRouterIdReuseDelay[aRouterId] == 0);

    mAllocatedRouterIds[aRouterId / 8] |= 1 << (aRouterId % 8);
    UpdateAllocation();

    rval = GetRouter(aRouterId);
    rval->SetLastHeard(TimerMilli::GetNow());

    mRouterIdSequence++;
    mRouterIdSequenceLastUpdated = TimerMilli::GetNow();
    GetNetif().GetMle().ResetAdvertiseInterval();

    otLogInfoMle(GetInstance(), "Allocate router id %d", aRouterId);

exit:
    return rval;
}