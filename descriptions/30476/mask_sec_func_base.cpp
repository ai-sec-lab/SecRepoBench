void Compiler::simplifyExpression(DefinitionMap& definitions,
                                  BasicBlock& b,
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
            if (!try_replace_expression(&b, iter, &optimized)) {
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
                var->storage() == Variable::Storage::kLocal && !definitions.get(var) &&
                optimizationContext->fSilences.find(var) == optimizationContext->fSilences.end()) {
                optimizationContext->fSilences.insert(var);
                this->error(expr->fOffset,
                            "'" + var->name() + "' has not been assigned");
            }
            break;
        }
        // <MASK>
    }
}