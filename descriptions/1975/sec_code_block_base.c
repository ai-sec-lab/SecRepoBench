psGXF->nGType = atoi(papszList[0]);
            if( psGXF->nGType < 0 || psGXF->nGType > 20 )
            {
                CSLDestroy( papszList );
                GXFClose( psGXF );
                return NULL;
            }