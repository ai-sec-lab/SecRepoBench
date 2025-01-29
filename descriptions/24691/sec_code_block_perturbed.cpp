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