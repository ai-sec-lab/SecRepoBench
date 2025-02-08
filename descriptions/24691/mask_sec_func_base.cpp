bool Compiler::optimize(Program& program) {
    SkASSERT(!fErrorCount);
    if (!program.fIsOptimized) {
        program.fIsOptimized = true;
        fIRGenerator->fKind = program.fKind;
        // <MASK>

        // Remove dead variables.
        if (program.fKind != Program::kFragmentProcessor_Kind) {
            for (auto iter = program.fElements.begin(); iter != program.fElements.end();) {
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
                        iter = program.fElements.erase(iter);
                        continue;
                    }
                }
                ++iter;
            }
        }
    }
    return fErrorCount == 0;
}