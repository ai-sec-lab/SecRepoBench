bool Compiler::optimize(Program& programCode) {
    SkASSERT(!fErrorCount);
    if (!programCode.fIsOptimized) {
        programCode.fIsOptimized = true;
        fIRGenerator->fKind = programCode.fKind;
        fIRGenerator->fSettings = &programCode.fSettings;

        // Build the control-flow graph for each function.
        for (ProgramElement& element : programCode) {
            if (element.fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG(element.as<FunctionDefinition>());
            }
        }

        // Remove dead functions. We wait until after analysis so that we still report errors, even
        // in unused code.
        if (programCode.fSettings.fRemoveDeadFunctions) {
            programCode.fElements.erase(
                    std::remove_if(programCode.fElements.begin(),
                                   programCode.fElements.end(),
                                   [](const std::unique_ptr<ProgramElement>& pe) {
                                       if (pe->fKind != ProgramElement::kFunction_Kind) {
                                           return false;
                                       }
                                       const FunctionDefinition& fn = pe->as<FunctionDefinition>();
                                       return fn.fDeclaration.fCallCount == 0 &&
                                              fn.fDeclaration.fName != "main";
                                   }),
                    programCode.fElements.end());
        }

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