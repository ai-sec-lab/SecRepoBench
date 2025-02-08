bool Compiler::optimize(Program& program) {
    SkASSERT(!fErrorCount);
    if (!program.fIsOptimized) {
        program.fIsOptimized = true;
        fIRGenerator->fKind = program.fKind;
        fIRGenerator->fSettings = &program.fSettings;

        // Build the control-flow graph for each function.
        for (ProgramElement& element : program) {
            if (element.fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG(element.as<FunctionDefinition>());
            }
        }

        // Remove dead functions. We wait until after analysis so that we still report errors, even
        // in unused code.
        if (program.fSettings.fRemoveDeadFunctions) {
            program.fElements.erase(
                    std::remove_if(program.fElements.begin(),
                                   program.fElements.end(),
                                   [](const std::unique_ptr<ProgramElement>& pe) {
                                       if (pe->fKind != ProgramElement::kFunction_Kind) {
                                           return false;
                                       }
                                       const FunctionDefinition& fn = pe->as<FunctionDefinition>();
                                       return fn.fDeclaration.fCallCount == 0 &&
                                              fn.fDeclaration.fName != "main";
                                   }),
                    program.fElements.end());
        }

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