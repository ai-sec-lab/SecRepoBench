// build index map
    for (uint8_t i = 0; i <= Mle::kMaxRouterId; i++)
    {
        if (IsAllocated(i) && mActiveRouterCount < Mle::kMaxRouters)
        {
            indexMap[i] = mActiveRouterCount++;
        }
        else
        {
            indexMap[i] = Mle::kInvalidRouterId;
        }
    }

    // shift entries forward
    for (int i = Mle::kMaxRouters - 2; i >= 0; i--)
    {
        uint8_t routerId = mRouters[i].GetRouterId();
        uint8_t newIndex;

        if (routerId > Mle::kMaxRouterId || indexMap[routerId] == Mle::kInvalidRouterId)
        {
            continue;
        }

        newIndex = indexMap[routerId];

        if (newIndex > i)
        {
            mRouters[newIndex] = mRouters[i];
        }
    }

    // shift entries backward
    for (uint8_t i = 1; i < Mle::kMaxRouters; i++)
    {
        uint8_t routerId = mRouters[i].GetRouterId();
        uint8_t newIndex;

        if (routerId > Mle::kMaxRouterId || indexMap[routerId] == Mle::kInvalidRouterId)
        {
            continue;
        }

        newIndex = indexMap[routerId];

        if (newIndex < i)
        {
            mRouters[newIndex] = mRouters[i];
        }
    }

    // fix replaced entries
    for (uint8_t i = 0; i <= Mle::kMaxRouterId; i++)
    {
        uint8_t index = indexMap[i];

        if (index != Mle::kInvalidRouterId)
        {
            Router &router = mRouters[index];

            if (router.GetRouterId() != i)
            {
                memset(&router, 0, sizeof(router));
                router.SetRloc16(Mle::Mle::GetRloc16(i));
                router.SetNextHop(Mle::kInvalidRouterId);
            }
        }
    }

    // clear unused entries
    for (uint8_t i = mActiveRouterCount; i < Mle::kMaxRouters; i++)
    {
        Router &router = mRouters[i];
        memset(&router, 0, sizeof(router));
        router.SetRloc16(0xffff);
    }