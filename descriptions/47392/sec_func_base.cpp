void Parser::ParseLV1SoftSkinBlock() {
    // TODO: fix line counting here

    // **************************************************************
    // The soft skin block is formatted differently. There are no
    // nested sections supported and the single elements aren't
    // marked by keywords starting with an asterisk.

    /**
    FORMAT BEGIN

    *MESH_SOFTSKINVERTS {
    <nodename>
    <number of vertices>

    [for <number of vertices> times:]
        <number of weights> [for <number of weights> times:] <bone name> <weight>
    }

    FORMAT END
    */
    // **************************************************************
    while (true) {
        if (*filePtr == '}') {
            ++filePtr;
            return;
        } else if (*filePtr == '\0')
            return;
        else if (*filePtr == '{')
            ++filePtr;

        else // if (!IsSpace(*filePtr) && !IsLineEnd(*filePtr))
        {
            ASE::Mesh *curMesh = nullptr;
            unsigned int numVerts = 0;

            const char *sz = filePtr;
            while (!IsSpaceOrNewLine(*filePtr))
                ++filePtr;

            const unsigned int diff = (unsigned int)(filePtr - sz);
            if (diff) {
                std::string name = std::string(sz, diff);
                for (std::vector<ASE::Mesh>::iterator it = m_vMeshes.begin();
                        it != m_vMeshes.end(); ++it) {
                    if ((*it).mName == name) {
                        curMesh = &(*it);
                        break;
                    }
                }
                if (!curMesh) {
                    LogWarning("Encountered unknown mesh in *MESH_SOFTSKINVERTS section");

                    // Skip the mesh data - until we find a new mesh
                    // or the end of the *MESH_SOFTSKINVERTS section
                    while (true) {
                        SkipSpacesAndLineEnd(&filePtr);
                        if (*filePtr == '}') {
                            ++filePtr;
                            return;
                        } else if (!IsNumeric(*filePtr))
                            break;

                        SkipLine(&filePtr);
                    }
                } else {
                    SkipSpacesAndLineEnd(&filePtr);
                    ParseLV4MeshLong(numVerts);

                    // Reserve enough storage
                    curMesh->mBoneVertices.reserve(numVerts);

                    for (unsigned int i = 0; i < numVerts; ++i) {
                        SkipSpacesAndLineEnd(&filePtr);
                        unsigned int numWeights;
                        ParseLV4MeshLong(numWeights);

                        curMesh->mBoneVertices.push_back(ASE::BoneVertex());
                        ASE::BoneVertex &vert = curMesh->mBoneVertices.back();

                        // Reserve enough storage
                        vert.mBoneWeights.reserve(numWeights);

                        std::string bone;
                        for (unsigned int w = 0; w < numWeights; ++w) {
                            bone.clear();
                            ParseString(bone, "*MESH_SOFTSKINVERTS.Bone");

                            // Find the bone in the mesh's list
                            std::pair<int, ai_real> me;
                            me.first = -1;

                            for (unsigned int n = 0; n < curMesh->mBones.size(); ++n) {
                                if (curMesh->mBones[n].mName == bone) {
                                    me.first = n;
                                    break;
                                }
                            }
                            if (-1 == me.first) {
                                // We don't have this bone yet, so add it to the list
                                me.first = static_cast<int>(curMesh->mBones.size());
                                curMesh->mBones.push_back(ASE::Bone(bone));
                            }
                            ParseLV4MeshFloat(me.second);

                            // Add the new bone weight to list
                            vert.mBoneWeights.push_back(me);
                        }
                    }
                }
            }
        }
        if (*filePtr == '\0')
            return;
        ++filePtr;
        SkipSpacesAndLineEnd(&filePtr);
    }
}