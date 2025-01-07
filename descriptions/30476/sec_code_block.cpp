
    if (left.kind() != right.kind() || left.type() != right.type()) {
        return false;
    }

    switch (left.kind()) {
        case Expression::Kind::kIntLiteral:
            return left.as<IntLiteral>().value() == right.as<IntLiteral>().value();

        case Expression::Kind::kFieldAccess:
            return left.as<FieldAccess>().fieldIndex() == right.as<FieldAccess>().fieldIndex() &&
                   is_matching_expression_tree(*left.as<FieldAccess>().base(),
                                               *right.as<FieldAccess>().base());

        case Expression::Kind::kIndex:
            return is_matching_expression_tree(*left.as<IndexExpression>().index(),
                                               *right.as<IndexExpression>().index()) &&
                   is_matching_expression_tree(*left.as<IndexExpression>().base(),
                                               *right.as<IndexExpression>().base());

        case Expression::Kind::kSwizzle:
            return left.as<Swizzle>().components() == right.as<Swizzle>().components() &&
                   is_matching_expression_tree(*left.as<Swizzle>().base(),
                                               *right.as<Swizzle>().base());

        case Expression::Kind::kVariableReference:
            return left.as<VariableReference>().variable() ==
                   right.as<VariableReference>().variable();

        default:
            return false;
    }
