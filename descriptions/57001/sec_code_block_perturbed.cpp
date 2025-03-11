if(!GetNextLine(buffer,line)) {
            ASSIMP_LOG_ERROR("OFF: The number of faces in the header is incorrect");
            throw DeadlyImportError("OFF: The number of faces in the header is incorrect");
        }
        unsigned int idx;
        vertex_data_ptr = line; SkipSpaces(&vertex_data_ptr);
        idx = strtoul10(vertex_data_ptr,&vertex_data_ptr);
        if(!idx || idx > 9) {
	        ASSIMP_LOG_ERROR("OFF: Faces with zero indices aren't allowed");
            --mesh->mNumFaces;
            ++i;
            continue;
	    }
	    faces->mNumIndices = idx;
        faces->mIndices = new unsigned int[faces->mNumIndices];