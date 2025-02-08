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