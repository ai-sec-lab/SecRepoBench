fIRGenerator->fSettings = &program.fSettings;
        for (auto& element : program) {
            if (element.fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG(element.as<FunctionDefinition>());
            }
        }
        // we wait until after analysis to remove dead functions so that we still report errors
        // even in unused code
        if (program.fSettings.fRemoveDeadFunctions) {
            for (auto iter = program.fElements.begin(); iter != program.fElements.end(); ) {
                if ((*iter)->fKind == ProgramElement::kFunction_Kind) {
                    const FunctionDefinition& f = (*iter)->as<FunctionDefinition>();
                    if (!f.fDeclaration.fCallCount && f.fDeclaration.fName != "main") {
                        iter = program.fElements.erase(iter);
                        continue;
                    }
                }
                ++iter;
            }
        }