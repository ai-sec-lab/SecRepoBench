bool Compiler::scanCFG(FunctionDefinition& f) {
    bool madeChanges = false;

    CFG cfg = CFGGenerator().getCFG(f);
    this->computeDataFlow(&cfg);

    // check for unreachable code
    for (size_t i = 0; i < cfg.fBlocks.size(); i++) {
        const BasicBlock& block = cfg.fBlocks[i];
        if (i != cfg.fStart && !block.fEntrances.size() && block.fNodes.size()) {
            int offset;
            const BasicBlock::Node& node = block.fNodes[0];
            switch (node.fKind) {
                case BasicBlock::Node::kStatement_Kind:
                    offset = (*node.statement())->fOffset;
                    break;
                case BasicBlock::Node::kExpression_Kind:
                    offset = (*node.expression())->fOffset;
                    if ((*node.expression())->is<BoolLiteral>()) {
                        // Function inlining can generate do { ... } while(false) loops which always
                        // break, so the boolean condition is considered unreachable. Since not
                        // being able to reach a literal is a non-issue in the first place, we
                        // don't report an error in this case.
                        continue;
                    }
                    break;
            }
            this->error(offset, String("unreachable"));
        }
    }
    if (fErrorCount) {
        return madeChanges;
    }

    // check for dead code & undefined variables, perform constant propagation
    std::unordered_set<const Variable*> undefinedVariables;
    bool updated;
    bool needsRescan = false;
    do {
        if (needsRescan) {
            cfg = CFGGenerator().getCFG(f);
            this->computeDataFlow(&cfg);
            needsRescan = false;
        }

        updated = false;
        bool first = true;
        for (BasicBlock& b : cfg.fBlocks) {
            if (!first && b.fEntrances.empty()) {
                // Block was reachable before optimization, but has since become unreachable. In
                // addition to being dead code, it's broken - since control flow can't reach it, no
                // prior variable definitions can reach it, and therefore variables might look to
                // have not been properly assigned. Kill it.

                // We need to do this in two steps. For any variable declarations, the node list
                // will contain statement nodes for each VarDeclaration, and then a statement for
                // the VarDeclarationsStatement. When we replace the VDS with a Nop, we delete the
                // storage of the unique_ptr that the VD nodes are pointing to. So we remove those
                // from the node list entirely, first.
                b.fNodes.erase(
                        std::remove_if(b.fNodes.begin(), b.fNodes.end(),
                                       [](const BasicBlock::Node& node) {
                                           return node.fKind == BasicBlock::Node::kStatement_Kind &&
                                                  (*node.statement())->is<VarDeclaration>();
                                       }),
                        b.fNodes.end());

                // Now replace any remaining statements in the block with Nops.
                for (BasicBlock::Node& node : b.fNodes) {
                    if (node.fKind == BasicBlock::Node::kStatement_Kind &&
                        !(*node.statement())->is<Nop>()) {
                        node.setStatement(std::make_unique<Nop>());
                        madeChanges = true;
                    }
                }
                continue;
            }
            first = false;
            DefinitionMap definitions = b.fBefore;

            for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan; ++iter) {
                if (iter->fKind == BasicBlock::Node::kExpression_Kind) {
                    this->simplifyExpression(definitions, b, &iter, &undefinedVariables, &updated,
                                             &needsRescan);
                } else {
                    this->simplifyStatement(definitions, b, &iter, &undefinedVariables, &updated,
                                            &needsRescan);
                }
                if (needsRescan) {
                    break;
                }
                this->addDefinitions(*iter, &definitions);
            }
        }
        madeChanges |= updated;
    } while (updated);
    SkASSERT(!needsRescan);

    // verify static ifs & switches, clean up dead variable decls
    for (BasicBlock& b : cfg.fBlocks) {
        DefinitionMap definitions = b.fBefore;

        for (auto iter = b.fNodes.begin(); iter != b.fNodes.end() && !needsRescan;) {
            if (iter->fKind == BasicBlock::Node::kStatement_Kind) {
                const Statement& s = **iter->statement();
                switch (s.kind()) {
                    case Statement::Kind::kIf:
                        if (s.as<IfStatement>().fIsStatic &&
                            !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static if has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::Kind::kSwitch:
                        if (s.as<SwitchStatement>().fIsStatic &&
                            !(fFlags & kPermitInvalidStaticTests_Flag)) {
                            this->error(s.fOffset, "static switch has non-static test");
                        }
                        ++iter;
                        break;
                    case Statement::Kind::kVarDeclarations: {
                        VarDeclarations& decls = *s.as<VarDeclarationsStatement>().fDeclaration;
                        decls.fVars.erase(
                                std::remove_if(decls.fVars.begin(), decls.fVars.end(),
                                               [&](const std::unique_ptr<Statement>& var) {
                                                   bool nop = var->is<Nop>();
                                                   madeChanges |= nop;
                                                   return nop;
                                               }),
                                decls.fVars.end());
                        if (decls.fVars.empty()) {
                            iter = b.fNodes.erase(iter);
                        } else {
                            ++iter;
                        }
                        break;
                    }
                    default:
                        ++iter;
                        break;
                }
            } else {
                ++iter;
            }
        }
    }

    // check for missing return
    if (f.fDeclaration.fReturnType != *fContext->fVoid_Type) {
        if (cfg.fBlocks[cfg.fExit].fEntrances.size()) {
            this->error(f.fOffset, String("function '" + String(f.fDeclaration.fName) +
                                          "' can exit without returning a value"));
        }
    }

    return madeChanges;
}