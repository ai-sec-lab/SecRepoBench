std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& left,
                                                      Token::Kind op,
                                                      const Expression& right) const {
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short circuit optimizations to allow the constant to be folded.
    if (left.is<BoolLiteral>() && !right.isCompileTimeConstant()) {
        return short_circuit_boolean(fContext, left, op, right);
    } else if (right.is<BoolLiteral>() && !left.isCompileTimeConstant()) {
        // There aren't side effects in SKSL within expressions, so (left OP right) is equivalent to
        // (right OP left) for short-circuit optimizations
        return short_circuit_boolean(fContext, right, op, left);
    }

    // Other than the short-circuit cases above, constant folding requires both sides to be constant
    if (!left.isCompileTimeConstant() || !right.isCompileTimeConstant()) {
        return nullptr;
    }
    // Note that we expressly do not worry about precision and overflow here -- we use the maximum
    // precision to calculate the results and hope the result makes sense. The plan is to move the
    // Skia caps into SkSL, so we have access to all of them including the precisions of the various
    // types, which will let us be more intelligent about this.
    if (left.is<BoolLiteral>() && right.is<BoolLiteral>()) {
        bool leftVal  = left.as<BoolLiteral>().value();
        bool rightVal = right.as<BoolLiteral>().value();
        bool result;
        switch (op) {
            case Token::Kind::TK_LOGICALAND: result = leftVal && rightVal; break;
            case Token::Kind::TK_LOGICALOR:  result = leftVal || rightVal; break;
            case Token::Kind::TK_LOGICALXOR: result = leftVal ^  rightVal; break;
            default: return nullptr;
        }
        return std::make_unique<BoolLiteral>(fContext, left.fOffset, result);
    }
    #define RESULT(t, op) std::make_unique<t ## Literal>(fContext, left.fOffset, \
                                                         leftVal op rightVal)
    #define URESULT(t, op) std::make_unique<t ## Literal>(fContext, left.fOffset, \
                                                          (uint32_t) leftVal op   \
                                                          (uint32_t) rightVal)
    if (left.is<IntLiteral>() && right.is<IntLiteral>()) {
        int64_t leftVal  = left.as<IntLiteral>().value();
        int64_t rightVal = right.as<IntLiteral>().value();
        switch (op) {
            case Token::Kind::TK_PLUS:       return URESULT(Int, +);
            case Token::Kind::TK_MINUS:      return URESULT(Int, -);
            case Token::Kind::TK_STAR:       return URESULT(Int, *);
            case Token::Kind::TK_SLASH:
                if (leftVal == std::numeric_limits<int64_t>::min() && rightVal == -1) {
                    fErrors.error(right.fOffset, "arithmetic overflow");
                    return nullptr;
                }
                if (!rightVal) {
                    fErrors.error(right.fOffset, "division by zero");
                    return nullptr;
                }
                return RESULT(Int, /);
            case Token::Kind::TK_PERCENT:
                if (leftVal == std::numeric_limits<int64_t>::min() && rightVal == -1) {
                    fErrors.error(right.fOffset, "arithmetic overflow");
                    return nullptr;
                }
                if (!rightVal) {
                    fErrors.error(right.fOffset, "division by zero");
                    return nullptr;
                }
                return RESULT(Int, %);
            case Token::Kind::TK_BITWISEAND: return RESULT(Int,  &);
            case Token::Kind::TK_BITWISEOR:  return RESULT(Int,  |);
            case Token::Kind::TK_BITWISEXOR: return RESULT(Int,  ^);
            case Token::Kind::TK_EQEQ:       return RESULT(Bool, ==);
            case Token::Kind::TK_NEQ:        return RESULT(Bool, !=);
            case Token::Kind::TK_GT:         return RESULT(Bool, >);
            case Token::Kind::TK_GTEQ:       return RESULT(Bool, >=);
            case Token::Kind::TK_LT:         return RESULT(Bool, <);
            case Token::Kind::TK_LTEQ:       return RESULT(Bool, <=);
            case Token::Kind::TK_SHL:
                if (rightVal >= 0 && rightVal <= 31) {
                    return URESULT(Int,  <<);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;
            case Token::Kind::TK_SHR:
                if (rightVal >= 0 && rightVal <= 31) {
                    return URESULT(Int,  >>);
                }
                fErrors.error(right.fOffset, "shift value out of range");
                return nullptr;

            default:
                return nullptr;
        }
    }
    if (left.is<FloatLiteral>() && right.is<FloatLiteral>()) {
        SKSL_FLOAT leftVal  = left.as<FloatLiteral>().value();
        SKSL_FLOAT rightVal = right.as<FloatLiteral>().value();
        switch (op) {
            case Token::Kind::TK_PLUS:  return RESULT(Float, +);
            case Token::Kind::TK_MINUS: return RESULT(Float, -);
            case Token::Kind::TK_STAR:  return RESULT(Float, *);
            case Token::Kind::TK_SLASH:
                if (rightVal) {
                    return RESULT(Float, /);
                }
                fErrors.error(right.fOffset, "division by zero");
                return nullptr;
            case Token::Kind::TK_EQEQ: return RESULT(Bool, ==);
            case Token::Kind::TK_NEQ:  return RESULT(Bool, !=);
            case Token::Kind::TK_GT:   return RESULT(Bool, >);
            case Token::Kind::TK_GTEQ: return RESULT(Bool, >=);
            case Token::Kind::TK_LT:   return RESULT(Bool, <);
            case Token::Kind::TK_LTEQ: return RESULT(Bool, <=);
            default:                   return nullptr;
        }
    }
    const Type& leftType = left.type();
    const Type& rightType = right.type();
    if (leftType.typeKind() == Type::TypeKind::kVector && leftType.componentType().isFloat() &&
        leftType == rightType) {
        ExpressionArray args;
        #define RETURN_VEC_COMPONENTWISE_RESULT(op)                                             \
            for (int i = 0; i < leftType.columns(); i++) {                                      \
                SKSL_FLOAT value = left.getFVecComponent(i) op right.getFVecComponent(i);       \
                args.push_back(std::make_unique<FloatLiteral>(fContext, left.fOffset, value));  \
            }                                                                                   \
            return std::make_unique<Constructor>(left.fOffset, &leftType, std::move(args))
        switch (op) {
            case Token::Kind::TK_EQEQ:
                if (left.kind() == right.kind()) {
                    return std::make_unique<BoolLiteral>(fContext, left.fOffset,
                                                         left.compareConstant(fContext, right));
                }
                return nullptr;
            case Token::Kind::TK_NEQ:
                if (left.kind() == right.kind()) {
                    return std::make_unique<BoolLiteral>(fContext, left.fOffset,
                                                         !left.compareConstant(fContext, right));
                }
                return nullptr;
            case Token::Kind::TK_PLUS:  RETURN_VEC_COMPONENTWISE_RESULT(+);
            case Token::Kind::TK_MINUS: RETURN_VEC_COMPONENTWISE_RESULT(-);
            case Token::Kind::TK_STAR:  RETURN_VEC_COMPONENTWISE_RESULT(*);
            case Token::Kind::TK_SLASH:
                for (int i = 0; i < leftType.columns(); i++) {
                    SKSL_FLOAT rvalue = right.getFVecComponent(i);
                    if (rvalue == 0.0) {
                        fErrors.error(right.fOffset, "division by zero");
                        return nullptr;
                    }
                    SKSL_FLOAT value = left.getFVecComponent(i) / rvalue;
                    args.push_back(std::make_unique<FloatLiteral>(fContext, /*offset=*/-1, value));
                }
                return std::make_unique<Constructor>(/*offset=*/-1, &leftType, std::move(args));
            default:
                return nullptr;
        }
        #undef RETURN_VEC_COMPONENTWISE_RESULT
    }
    if (leftType.typeKind() == Type::TypeKind::kMatrix &&
        rightType.typeKind() == Type::TypeKind::kMatrix &&
        left.kind() == right.kind()) {
        switch (op) {
            case Token::Kind::TK_EQEQ:
                return std::make_unique<BoolLiteral>(fContext, left.fOffset,
                                                     left.compareConstant(fContext, right));
            case Token::Kind::TK_NEQ:
                return std::make_unique<BoolLiteral>(fContext, left.fOffset,
                                                     !left.compareConstant(fContext, right));
            default:
                return nullptr;
        }
    }
    #undef RESULT
    return nullptr;
}