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