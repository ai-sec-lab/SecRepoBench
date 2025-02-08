std::unique_ptr<Expression> IRGenerator::constantFold(const Expression& lhs,
                                                      Token::Kind op,
                                                      const Expression& right) const {
    // If the left side is a constant boolean literal, the right side does not need to be constant
    // for short circuit optimizations to allow the constant to be folded.
    // <MASK>
    #undef RESULT
    return nullptr;
}