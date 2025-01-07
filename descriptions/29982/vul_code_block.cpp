
                    // We found at least one argument that could be flattened out. Re-walk the
                    // constructor args and flatten the candidates we found during our initial pass.
                    ExpressionArray flattened;
                    flattened.reserve_back(c.type().columns());
                    currBit = 1;
                    for (const std::unique_ptr<Expression>& arg : c.arguments()) {
                        if (argsToOptimize & currBit) {
                            Constructor& inner = arg->as<Constructor>();
                            for (const std::unique_ptr<Expression>& innerArg : inner.arguments()) {
                                flattened.push_back(innerArg->clone());
                            }
                        } else {
                            flattened.push_back(arg->clone());
                        }
                        currBit <<= 1;
                    }
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
                