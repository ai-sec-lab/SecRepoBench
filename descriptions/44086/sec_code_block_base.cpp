if (iIndex < mat.avSubMaterials.size()) {
                    Material &sMat = mat.avSubMaterials[iIndex];

                    // parse the material block
                    ParseLV2MaterialBlock(sMat);
                }

                continue;