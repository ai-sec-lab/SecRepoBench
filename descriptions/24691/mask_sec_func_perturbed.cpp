bool Compiler::optimize(Program& programCode) {
    SkASSERT(!fErrorCount);
    if (!programCode.fIsOptimized) {
        programCode.fIsOptimized = true;
        fIRGenerator->fKind = programCode.fKind;
        // <MASK>

        // Remove dead variables.
        if (programCode.fKind != Program::kFragmentProcessor_Kind) {
            for (auto iter = programCode.fElements.begin(); iter != programCode.fElements.end();) {
                if ((*iter)->fKind == ProgramElement::kVar_Kind) {
                    VarDeclarations& vars = (*iter)->as<VarDeclarations>();
                    for (auto varIter = vars.fVars.begin(); varIter != vars.fVars.end();) {
                        const Variable& var = *(*varIter)->as<VarDeclaration>().fVar;
                        if (var.dead()) {
                            varIter = vars.fVars.erase(varIter);
                        } else {
                            ++varIter;
                        }
                    }
                    if (vars.fVars.size() == 0) {
                        iter = programCode.fElements.erase(iter);
                        continue;
                    }
                }
                ++iter;
            }
        }
    }
    return fErrorCount == 0;
}