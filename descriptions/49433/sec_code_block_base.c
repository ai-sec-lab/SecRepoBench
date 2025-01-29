// Check types. Abort whole profile if a forged link is found
                if (CompatibleTypes(_cmsGetTagDescriptor(Icc->ContextID, Icc->TagNames[j]),
                                    _cmsGetTagDescriptor(Icc->ContextID, Tag.sig))) {

                    Icc->TagLinked[Icc->TagCount] = Icc->TagNames[j];
                }