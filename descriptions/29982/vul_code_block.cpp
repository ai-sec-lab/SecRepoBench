auto optimized = std::unique_ptr<Expression>(
                            new Constructor(c.fOffset, &c.type(), std::move(flattened)));
                    // No fUsage change; no references have been added or removed anywhere.
                    optimizationContext->fUpdated = true;
                    if (!try_replace_expression(&b, iter, &optimized)) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;