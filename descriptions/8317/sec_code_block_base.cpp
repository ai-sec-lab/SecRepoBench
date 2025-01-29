Router *rval = NULL;

    VerifyOrExit(aRouterId <= Mle::kMaxRouterId && !IsAllocated(aRouterId) && mRouterIdReuseDelay[aRouterId] == 0);

    mAllocatedRouterIds[aRouterId / 8] |= 1 << (aRouterId % 8);
    UpdateAllocation();