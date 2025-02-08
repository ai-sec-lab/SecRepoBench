if( psDef->nSize < 0 )
            {
                CPLError(CE_Failure, CPLE_AppDefined,
                 "Error parsing E00 Table Definition line: \"%s\"", pszLine);
                psInfo->numItems = psInfo->iCurItem = 0;
                return nullptr;
            }

            psInfo->nCurObjectId++;