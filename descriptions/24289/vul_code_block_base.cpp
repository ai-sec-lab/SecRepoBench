for (BasicBlock::Node& node : b.fNodes) {
                    if (node.fKind == BasicBlock::Node::kStatement_Kind &&
                        !(*node.statement())->is<Nop>()) {
                        node.setStatement(std::make_unique<Nop>());
                        madeChanges = true;
                    }
                }
                continue;