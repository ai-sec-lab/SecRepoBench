fExtraFunctions.printf("%s", rowSeparator);
            rowSeparator = ", ";

            const Type& argType = args[argIndex]->fType;
            switch (argType.kind()) {
                case Type::kScalar_Kind: {
                    fExtraFunctions.printf("x%zu", argIndex);
                    break;
                }
                case Type::kVector_Kind: {
                    fExtraFunctions.printf("x%zu[%d]", argIndex, argPosition);
                    break;
                }
                case Type::kMatrix_Kind: {
                    fExtraFunctions.printf("x%zu[%d][%d]", argIndex,
                                           argPosition / argType.rows(),
                                           argPosition % argType.rows());
                    break;
                }
                default: {
                    SkDEBUGFAIL("incorrect type of argument for matrix constructor");
                    fExtraFunctions.printf("<error>");
                    break;
                }
            }

            ++argPosition;
            if (argPosition >= argType.columns() * argType.rows()) {
                ++argIndex;
                argPosition = 0;
            }