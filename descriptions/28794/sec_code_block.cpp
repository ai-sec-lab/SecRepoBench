
                Constructor& base = s.base()->as<Constructor>();
                std::unique_ptr<Expression> replacement;
                const Type& componentType = base.type().componentType();
                int swizzleSize = s.components().size();

                // The IR generator has already converted any zero/one swizzle components into
                // constructors containing zero/one args. Confirm that this is true by checking that
                // our swizzle components are all `xyzw` (values 0 through 3).
                SkASSERT(std::all_of(s.components().begin(), s.components().end(),
                                     [](int8_t c) { return c >= 0 && c <= 3; }));

                if (base.arguments().size() == 1 && base.arguments().front()->type().isScalar()) {
                    // `half4(scalar).zyy` can be optimized to `half3(scalar)`. The swizzle
                    // components don't actually matter since all fields are the same.
                    ExpressionArray newArgs;
                    newArgs.push_back(base.arguments().front()->clone());
                    replacement = std::make_unique<Constructor>(
                            base.fOffset,
                            &componentType.toCompound(*fContext, swizzleSize, /*rows=*/1),
                            std::move(newArgs));

                    // We're replacing an expression with a cloned version; we'll need a rescan.
                    // There's no fUsage change: `half4(foo).xy` and `half2(foo)` have equivalent
                    // reference counts.
                    try_replace_expression(&b, iter, &replacement);
                    optimizationContext->fUpdated = true;
                    optimizationContext->fNeedsRescan = true;
                    break;
                }

                // Swizzles can duplicate some elements and discard others, e.g.
                // `half4(1, 2, 3, 4).xxz` --> `half3(1, 1, 3)`. However, there are constraints:
                // - Expressions with side effects need to occur exactly once, even if they
                //   would otherwise be swizzle-eliminated
                // - Non-trivial expressions should not be repeated, but elimination is OK.
                //
                // Look up the argument for the constructor at each index. This is typically simple
                // but for weird cases like `half4(bar.yz, half2(foo))`, it can be harder than it
                // seems. This example would result in:
                //     argMap[0] = {.fArgIndex = 0, .fComponent = 0}   (bar.yz     .x)
                //     argMap[1] = {.fArgIndex = 0, .fComponent = 1}   (bar.yz     .y)
                //     argMap[2] = {.fArgIndex = 1, .fComponent = 0}   (half2(foo) .x)
                //     argMap[3] = {.fArgIndex = 1, .fComponent = 1}   (half2(foo) .y)
                struct ConstructorArgMap {
                    int8_t fArgIndex;
                    int8_t fComponent;
                };

                int numConstructorArgs = base.type().columns();
                ConstructorArgMap argMap[4] = {};
                int writeIdx = 0;
                for (int argIdx = 0; argIdx < (int) base.arguments().size(); ++argIdx) {
                    const Expression& expr = *base.arguments()[argIdx];
                    int argWidth = expr.type().columns();
                    for (int componentIdx = 0; componentIdx < argWidth; ++componentIdx) {
                        argMap[writeIdx].fArgIndex = argIdx;
                        argMap[writeIdx].fComponent = componentIdx;
                        ++writeIdx;
                    }
                }
                SkASSERT(writeIdx == numConstructorArgs);

                // Count up the number of times each constructor argument is used by the
                // swizzle.
                //    `half4(bar.yz, half2(foo)).xwxy` -> { 3, 1 }
                // - bar.yz    is referenced 3 times, by `.x_xy`
                // - half(foo) is referenced 1 time,  by `._w__`
                int8_t exprUsed[4] = {};
                for (int c : s.components()) {
                    exprUsed[argMap[c].fArgIndex]++;
                }

                bool safeToOptimize = true;
                for (int index = 0; index < numConstructorArgs; ++index) {
                    int8_t constructorArgIndex = argMap[index].fArgIndex;
                    const Expression& baseArg = *base.arguments()[constructorArgIndex];

                    // Check that non-trivial expressions are not swizzled in more than once.
                    if (exprUsed[constructorArgIndex] > 1 &&
                            !Analysis::IsTrivialExpression(baseArg)) {
                        safeToOptimize = false;
                        break;
                    }
                    // Check that side-effect-bearing expressions are swizzled in exactly once.
                    if (exprUsed[constructorArgIndex] != 1 && baseArg.hasSideEffects()) {
                        safeToOptimize = false;
                        break;
                    }
                }

                if (safeToOptimize) {
                    struct ReorderedArgument {
                        int8_t fArgIndex;
                        ComponentArray fComponents;
                    };
                    SkSTArray<4, ReorderedArgument> reorderedArgs;
                    for (int c : s.components()) {
                        const ConstructorArgMap& argument = argMap[c];
                        const Expression& baseArg = *base.arguments()[argument.fArgIndex];

                        if (baseArg.type().isScalar()) {
                            // This argument is a scalar; add it to the list as-is.
                            SkASSERT(argument.fComponent == 0);
                            reorderedArgs.push_back({argument.fArgIndex,
                                                     ComponentArray{}});
                        } else {
                            // This argument is a component from a vector.
                            SkASSERT(argument.fComponent < baseArg.type().columns());
                            if (reorderedArgs.empty() ||
                                reorderedArgs.back().fArgIndex != argument.fArgIndex) {
                                // This can't be combined with the previous argument. Add a new one.
                                reorderedArgs.push_back({argument.fArgIndex,
                                                         ComponentArray{argument.fComponent}});
                            } else {
                                // Since we know this argument uses components, it should already
                                // have at least one component set.
                                SkASSERT(!reorderedArgs.back().fComponents.empty());
                                // Build up the current argument with one more component.
                                reorderedArgs.back().fComponents.push_back(argument.fComponent);
                            }
                        }
                    }

                    // Convert our reordered argument list to an actual array of expressions, with
                    // the new order and any new inner swizzles that need to be applied. Note that
                    // we expect followup passes to clean up the inner swizzles.
                    ExpressionArray newArgs;
                    newArgs.reserve_back(swizzleSize);
                    for (const ReorderedArgument& reorderedArg : reorderedArgs) {
                        const Expression& baseArg = *base.arguments()[reorderedArg.fArgIndex];
                        if (reorderedArg.fComponents.empty()) {
                            newArgs.push_back(baseArg.clone());
                        } else {
                            newArgs.push_back(std::make_unique<Swizzle>(*fContext, baseArg.clone(),
                                                                        reorderedArg.fComponents));
                        }
                    }

                    // Create a new constructor.
                    replacement = std::make_unique<Constructor>(
                            base.fOffset,
                            &componentType.toCompound(*fContext, swizzleSize, /*rows=*/1),
                            std::move(newArgs));

                    // Remove references within 'expr', add references within 'replacement.'
                    optimizationContext->fUsage->replace(expr, replacement.get());

                    // We're replacing an expression with a cloned version; we'll need a rescan.
                    try_replace_expression(&b, iter, &replacement);
                    optimizationContext->fUpdated = true;
                    optimizationContext->fNeedsRescan = true;
                }
                break;
            