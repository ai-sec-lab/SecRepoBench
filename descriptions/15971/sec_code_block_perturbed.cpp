if (borderRouter)
                {
                    BorderRouterTlv *borderRouterBase = FindBorderRouter(*prefixBase, true);

                    if (!borderRouterBase || (borderRouter->GetLength() != borderRouterBase->GetLength()) ||
                        (memcmp(borderRouter, borderRouterBase, borderRouter->GetLength()) != 0))
                    {
                        ExitNow(rval = true);
                    }
                }

                if (hasRoute)
                {
                    HasRouteTlv *hasRouteBase = FindHasRoute(*prefixBase, true);

                    if (!hasRouteBase || (hasRoute->GetLength() != hasRouteBase->GetLength()) ||
                        (memcmp(hasRoute, hasRouteBase, hasRoute->GetLength()) != 0))
                    {
                        ExitNow(rval = true);
                    }
                }