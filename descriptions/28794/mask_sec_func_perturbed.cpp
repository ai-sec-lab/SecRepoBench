void Compiler::simplifyExpression(DefinitionMap& definitions,
                                  BasicBlock& basicBlock,
                                  std::vector<BasicBlock::Node>::iterator* iter,
                                  OptimizationContext* optimizationContext) {
    Expression* expr = (*iter)->expression()->get();
    SkASSERT(expr);

    if ((*iter)->fConstantPropagation) {
        std::unique_ptr<Expression> optimized = expr->constantPropagate(*fIRGenerator,
                                                                        definitions);
        if (optimized) {
            optimizationContext->fUpdated = true;
            optimized = fIRGenerator->coerce(std::move(optimized), expr->type());
            SkASSERT(optimized);
            // Remove references within 'expr', add references within 'optimized'
            optimizationContext->fUsage->replace(expr, optimized.get());
            if (!try_replace_expression(&basicBlock, iter, &optimized)) {
                optimizationContext->fNeedsRescan = true;
                return;
            }
            SkASSERT((*iter)->isExpression());
            expr = (*iter)->expression()->get();
        }
    }
    switch (expr->kind()) {
        case Expression::Kind::kVariableReference: {
            const VariableReference& ref = expr->as<VariableReference>();
            const Variable* var = ref.variable();
            if (ref.refKind() != VariableReference::RefKind::kWrite &&
                ref.refKind() != VariableReference::RefKind::kPointer &&
                var->storage() == Variable::Storage::kLocal && !definitions[var] &&
                optimizationContext->fSilences.find(var) == optimizationContext->fSilences.end()) {
                optimizationContext->fSilences.insert(var);
                this->error(expr->fOffset,
                            "'" + var->name() + "' has not been assigned");
            }
            break;
        }
        case Expression::Kind::kTernary: {
            TernaryExpression* t = &expr->as<TernaryExpression>();
            if (t->test()->is<BoolLiteral>()) {
                // ternary has a constant test, replace it with either the true or
                // false branch
                if (t->test()->as<BoolLiteral>().value()) {
                    (*iter)->setExpression(std::move(t->ifTrue()), optimizationContext->fUsage);
                } else {
                    (*iter)->setExpression(std::move(t->ifFalse()), optimizationContext->fUsage);
                }
                optimizationContext->fUpdated = true;
                optimizationContext->fNeedsRescan = true;
            }
            break;
        }
        case Expression::Kind::kBinary: {
            BinaryExpression* bin = &expr->as<BinaryExpression>();
            if (dead_assignment(*bin, optimizationContext->fUsage)) {
                delete_left(&basicBlock, iter, optimizationContext);
                break;
            }
            Expression& left = *bin->left();
            Expression& right = *bin->right();
            const Type& leftType = left.type();
            const Type& rightType = right.type();
            // collapse useless expressions like x * 1 or x + 0
            if ((!leftType.isScalar() && !leftType.isVector()) ||
                (!rightType.isScalar() && !rightType.isVector())) {
                break;
            }
            switch (bin->getOperator()) {
                case Token::Kind::TK_STAR:
                    if (is_constant(left, 1)) {
                        if (leftType.isVector() && rightType.isScalar()) {
                            // float4(1) * x -> float4(x)
                            vectorize_right(&basicBlock, iter, optimizationContext);
                        } else {
                            // 1 * x -> x
                            // 1 * float4(x) -> float4(x)
                            // float4(1) * float4(x) -> float4(x)
                            delete_left(&basicBlock, iter, optimizationContext);
                        }
                    }
                    else if (is_constant(left, 0)) {
                        if (leftType.isScalar() && rightType.isVector() &&
                            !right.hasSideEffects()) {
                            // 0 * float4(x) -> float4(0)
                            vectorize_left(&basicBlock, iter, optimizationContext);
                        } else {
                            // 0 * x -> 0
                            // float4(0) * x -> float4(0)
                            // float4(0) * float4(x) -> float4(0)
                            if (!right.hasSideEffects()) {
                                delete_right(&basicBlock, iter, optimizationContext);
                            }
                        }
                    }
                    else if (is_constant(right, 1)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x * float4(1) -> float4(x)
                            vectorize_left(&basicBlock, iter, optimizationContext);
                        } else {
                            // x * 1 -> x
                            // float4(x) * 1 -> float4(x)
                            // float4(x) * float4(1) -> float4(x)
                            delete_right(&basicBlock, iter, optimizationContext);
                        }
                    }
                    else if (is_constant(right, 0)) {
                        if (leftType.isVector() && rightType.isScalar() && !left.hasSideEffects()) {
                            // float4(x) * 0 -> float4(0)
                            vectorize_right(&basicBlock, iter, optimizationContext);
                        } else {
                            // x * 0 -> 0
                            // x * float4(0) -> float4(0)
                            // float4(x) * float4(0) -> float4(0)
                            if (!left.hasSideEffects()) {
                                delete_left(&basicBlock, iter, optimizationContext);
                            }
                        }
                    }
                    break;
                case Token::Kind::TK_PLUS:
                    if (is_constant(left, 0)) {
                        if (leftType.isVector() && rightType.isScalar()) {
                            // float4(0) + x -> float4(x)
                            vectorize_right(&basicBlock, iter, optimizationContext);
                        } else {
                            // 0 + x -> x
                            // 0 + float4(x) -> float4(x)
                            // float4(0) + float4(x) -> float4(x)
                            delete_left(&basicBlock, iter, optimizationContext);
                        }
                    } else if (is_constant(right, 0)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x + float4(0) -> float4(x)
                            vectorize_left(&basicBlock, iter, optimizationContext);
                        } else {
                            // x + 0 -> x
                            // float4(x) + 0 -> float4(x)
                            // float4(x) + float4(0) -> float4(x)
                            delete_right(&basicBlock, iter, optimizationContext);
                        }
                    }
                    break;
                case Token::Kind::TK_MINUS:
                    if (is_constant(right, 0)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x - float4(0) -> float4(x)
                            vectorize_left(&basicBlock, iter, optimizationContext);
                        } else {
                            // x - 0 -> x
                            // float4(x) - 0 -> float4(x)
                            // float4(x) - float4(0) -> float4(x)
                            delete_right(&basicBlock, iter, optimizationContext);
                        }
                    }
                    break;
                case Token::Kind::TK_SLASH:
                    if (is_constant(right, 1)) {
                        if (leftType.isScalar() && rightType.isVector()) {
                            // x / float4(1) -> float4(x)
                            vectorize_left(&basicBlock, iter, optimizationContext);
                        } else {
                            // x / 1 -> x
                            // float4(x) / 1 -> float4(x)
                            // float4(x) / float4(1) -> float4(x)
                            delete_right(&basicBlock, iter, optimizationContext);
                        }
                    } else if (is_constant(left, 0)) {
                        if (leftType.isScalar() && rightType.isVector() &&
                            !right.hasSideEffects()) {
                            // 0 / float4(x) -> float4(0)
                            vectorize_left(&basicBlock, iter, optimizationContext);
                        } else {
                            // 0 / x -> 0
                            // float4(0) / x -> float4(0)
                            // float4(0) / float4(x) -> float4(0)
                            if (!right.hasSideEffects()) {
                                delete_right(&basicBlock, iter, optimizationContext);
                            }
                        }
                    }
                    break;
                case Token::Kind::TK_PLUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&basicBlock, iter, optimizationContext);
                    }
                    break;
                case Token::Kind::TK_MINUSEQ:
                    if (is_constant(right, 0)) {
                        clear_write(left);
                        delete_right(&basicBlock, iter, optimizationContext);
                    }
                    break;
                case Token::Kind::TK_STAREQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&basicBlock, iter, optimizationContext);
                    }
                    break;
                case Token::Kind::TK_SLASHEQ:
                    if (is_constant(right, 1)) {
                        clear_write(left);
                        delete_right(&basicBlock, iter, optimizationContext);
                    }
                    break;
                default:
                    break;
            }
            break;
        }
        case Expression::Kind::kConstructor: {
            // Find constructors embedded inside constructors and flatten them out where possible.
            //   -  float4(float2(1, 2), 3, 4)                -->  float4(1, 2, 3, 4)
            //   -  float4(w, float3(sin(x), cos(y), tan(z))) -->  float4(w, sin(x), cos(y), tan(z))
            // Leave single-argument constructors alone, though. These might be casts or splats.
            Constructor& c = expr->as<Constructor>();
            if (c.type().columns() > 1) {
                // Inspect each constructor argument to see if it's a candidate for flattening.
                // Remember matched arguments in a bitfield, "argsToOptimize".
                int argsToOptimize = 0;
                int currBit = 1;
                for (const std::unique_ptr<Expression>& arg : c.arguments()) {
                    if (arg->is<Constructor>()) {
                        Constructor& inner = arg->as<Constructor>();
                        if (inner.arguments().size() > 1 &&
                            inner.type().componentType() == c.type().componentType()) {
                            argsToOptimize |= currBit;
                        }
                    }
                    currBit <<= 1;
                }
                if (argsToOptimize) {
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
                    if (!try_replace_expression(&basicBlock, iter, &optimized)) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;
                }
            }
            break;
        }
        case Expression::Kind::kSwizzle: {
            Swizzle& s = expr->as<Swizzle>();
            // Detect identity swizzles like `foo.rgba`.
            if ((int) s.components().size() == s.base()->type().columns()) {
                bool identity = true;
                for (int i = 0; i < (int) s.components().size(); ++i) {
                    if (s.components()[i] != i) {
                        identity = false;
                        break;
                    }
                }
                if (identity) {
                    optimizationContext->fUpdated = true;
                    // No fUsage change: foo.rgba and foo have equivalent reference counts
                    if (!try_replace_expression(&basicBlock, iter, &s.base())) {
                        optimizationContext->fNeedsRescan = true;
                        return;
                    }
                    SkASSERT((*iter)->isExpression());
                    break;
                }
            }
            // Detect swizzles of swizzles, e.g. replace `foo.argb.r000` with `foo.a000`.
            if (s.base()->is<Swizzle>()) {
                Swizzle& base = s.base()->as<Swizzle>();
                ComponentArray final;
                for (int c : s.components()) {
                    final.push_back(base.components()[c]);
                }
                optimizationContext->fUpdated = true;
                std::unique_ptr<Expression> replacement(new Swizzle(*fContext, base.base()->clone(),
                                                                    final));
                // No fUsage change: `foo.gbr.gbr` and `foo.brg` have equivalent reference counts
                if (!try_replace_expression(&basicBlock, iter, &replacement)) {
                    optimizationContext->fNeedsRescan = true;
                    return;
                }
                SkASSERT((*iter)->isExpression());
                break;
            }
            // Optimize swizzles of constructors.
            if (s.base()->is<Constructor>()) {
                Constructor& base = s.base()->as<Constructor>();
                std::unique_ptr<Expression> replacement;
                const Type& componentType = base.type().componentType();
                int swizzleSize = s.components().size();

                // The IR generator has already converted any zero/one swizzle components into
                // constructors containing zero/one args. Confirm that this is true by checking that
                // our swizzle components are all `xyzw` (values 0 through 3).
                SkASSERT(std::all_of(s.components().begin(), s.components().end(),
                                     [](int8_t c) { return c >= 0 && c <= 3; }));

                // <MASK>
                break;
            }
            break;
        }
        default:
            break;
    }
}