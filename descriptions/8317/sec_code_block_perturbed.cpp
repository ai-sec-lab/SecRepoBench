Router *rval = NULL;

    VerifyOrExit(routerId <= Mle::kMaxRouterId && !IsAllocated(routerId) && mRouterIdReuseDelay[routerId] == 0);

    mAllocatedRouterIds[routerId / 8] |= 1 << (routerId % 8);
    UpdateAllocation();