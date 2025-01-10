
        if(!GetNextLine(buffer,line)) {
            ASSIMP_LOG_ERROR("OFF: The number of faces in the header is incorrect");
            throw DeadlyImportError("OFF: The number of faces in the header is incorrect");
        }
        unsigned int idx;
        sz = line; SkipSpaces(&sz);
        idx = strtoul10(sz,&sz);
        if(!idx || idx > 9) {
	        ASSIMP_LOG_ERROR("OFF: Faces with zero indices aren't allowed");
            --mesh->mNumFaces;
            ++i;
            continue;
	    }
	    faces->mNumIndices = idx;
        faces->mIndices = new unsigned int[faces->mNumIndices];
        for (unsigned int m = 0; m < faces->mNumIndices;++m) {
            SkipSpaces(&sz);
            idx = strtoul10(sz,&sz);
            if (idx >= numVertices) {
                ASSIMP_LOG_ERROR("OFF: Vertex index is out of range");
                idx = numVertices - 1;
            }
            faces->mIndices[m] = idx;
        }
        ++i;
        ++faces;
    