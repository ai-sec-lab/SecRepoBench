fIRGenerator->fSettings = &programCode.fSettings;
        for (auto& element : programCode) {
            if (element.fKind == ProgramElement::kFunction_Kind) {
                this->scanCFG(element.as<FunctionDefinition>());
            }
        }
        // we wait until after analysis to remove dead functions so that we still report errors
        // even in unused code
        if (programCode.fSettings.fRemoveDeadFunctions) {
            for (auto iter = programCode.fElements.begin(); iter != programCode.fElements.end(); ) {
                if ((*iter)->fKind == ProgramElement::kFunction_Kind) {
                    const FunctionDefinition& f = (*iter)->as<FunctionDefinition>();
                    if (!f.fDeclaration.fCallCount && f.fDeclaration.fName != "main") {
                        iter = programCode.fElements.erase(iter);
                        continue;
                    }
                }
                ++iter;
            }
        }