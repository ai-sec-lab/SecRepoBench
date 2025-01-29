// build index map
    for (uint8_t index = 0; index <= Mle::kMaxRouterId; index++)
    {
        if (IsAllocated(index) && mActiveRouterCount < Mle::kMaxRouters)
        {
            indexMap[index] = mActiveRouterCount++;
        }
        else
        {
            indexMap[index] = Mle::kInvalidRouterId;
        }
    }

    // shift entries forward
    for (int index = Mle::kMaxRouters - 2; index >= 0; index--)
    {
        uint8_t routerId = mRouters[index].GetRouterId();
        uint8_t newIndex;

        if (routerId > Mle::kMaxRouterId || indexMap[routerId] == Mle::kInvalidRouterId)
        {
            continue;
        }

        newIndex = indexMap[routerId];

        if (newIndex > index)
        {
            mRouters[newIndex] = mRouters[index];
        }
    }

    // shift entries backward
    for (uint8_t index = 1; index < Mle::kMaxRouters; index++)
    {
        uint8_t routerId = mRouters[index].GetRouterId();
        uint8_t newIndex;

        if (routerId > Mle::kMaxRouterId || indexMap[routerId] == Mle::kInvalidRouterId)
        {
            continue;
        }

        newIndex = indexMap[routerId];

        if (newIndex < index)
        {
            mRouters[newIndex] = mRouters[index];
        }
    }

    // fix replaced entries
    for (uint8_t index = 0; index <= Mle::kMaxRouterId; index++)
    {
        uint8_t index = indexMap[index];

        if (index != Mle::kInvalidRouterId)
        {
            Router &router = mRouters[index];

            if (router.GetRouterId() != index)
            {
                memset(&router, 0, sizeof(router));
                router.SetRloc16(Mle::Mle::GetRloc16(index));
                router.SetNextHop(Mle::kInvalidRouterId);
            }
        }
    }

    // clear unused entries
    for (uint8_t index = mActiveRouterCount; index < Mle::kMaxRouters; index++)
    {
        Router &router = mRouters[index];
        memset(&router, 0, sizeof(router));
        router.SetRloc16(0xffff);
    }