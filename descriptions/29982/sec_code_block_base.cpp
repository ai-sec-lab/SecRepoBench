std::unique_ptr<Expression> replacement(new Constructor(c.fOffset, &c.type(),
                                                                            std::move(flattened)));
                    // We're replacing an expression with a cloned version; we'll need a rescan.
                    // No fUsage change: `float2(float(x), y)` and `float2(x, y)` have equivalent
                    // reference counts.
                    try_replace_expression(&b, iter, &replacement);
                    optimizationContext->fUpdated = true;
                    optimizationContext->fNeedsRescan = true;
                    break;