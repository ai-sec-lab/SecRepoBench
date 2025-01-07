
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
                    std::unique_ptr<Expression> replacement(new Constructor(c.fOffset, &c.type(),
                                                                            std::move(flattened)));
                    // We're replacing an expression with a cloned version; we'll need a rescan.
                    // No fUsage change: `float2(float(x), y)` and `float2(x, y)` have equivalent
                    // reference counts.
                    try_replace_expression(&b, iter, &replacement);
                    optimizationContext->fUpdated = true;
                    optimizationContext->fNeedsRescan = true;
                    break;
                