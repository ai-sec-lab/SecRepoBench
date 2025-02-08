String MetalCodeGenerator::getMatrixConstructHelper(const Constructor& c) {
    const Type& matrix = c.fType;
    int columns = matrix.columns();
    int rows = matrix.rows();
    const std::vector<std::unique_ptr<Expression>>& args = c.fArguments;

    // Create the helper-method name and use it as our lookup key.
    String name;
    name.appendf("float%dx%d_from", columns, rows);
    for (const std::unique_ptr<Expression>& expr : args) {
        name.appendf("_%s", expr->fType.displayName().c_str());
    }

    // If a helper-method has already been synthesized, we don't need to synthesize it again.
    auto [iter, newlyCreated] = fHelpers.insert(name);
    if (!newlyCreated) {
        return name;
    }

    // Unlike GLSL, Metal requires that matrices are initialized with exactly R vectors of C
    // components apiece. (In Metal 2.0, you can also supply R*C scalars, but you still cannot
    // supply a mixture of scalars and vectors.)
    fExtraFunctions.printf("float%dx%d %s(", columns, rows, name.c_str());

    size_t argIndex = 0;
    const char* argSeparator = "";
    for (const std::unique_ptr<Expression>& expr : c.fArguments) {
        fExtraFunctions.printf("%s%s x%zu", argSeparator,
                               expr->fType.displayName().c_str(), argIndex++);
        argSeparator = ", ";
    }

    fExtraFunctions.printf(") {\n    return float%dx%d(", columns, rows);

    argIndex = 0;
    int argPosition = 0;

    const char* columnSeparator = "";
    for (int c = 0; c < columns; ++c) {
        fExtraFunctions.printf("%sfloat%d(", columnSeparator, rows);
        columnSeparator = "), ";

        const char* rowSeparator = "";
        for (int r = 0; r < rows; ++r) {
            fExtraFunctions.printf("%s", rowSeparator);
            rowSeparator = ", ";

            if (argIndex < args.size()) {
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
            } else {
                SkDEBUGFAIL("not enough arguments for matrix constructor");
                fExtraFunctions.printf("<error>");
            }
        }
    }

    if (argPosition != 0 || argIndex != args.size()) {
        SkDEBUGFAIL("incorrect number of arguments for matrix constructor");
        name = "<error>";
    }

    fExtraFunctions.printf("));\n}\n");
    return name;
}