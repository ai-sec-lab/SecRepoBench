poGeomToDelete = std::unique_ptr<OGRGeometry>(
                OGRGeometryFactory::forceToMultiPolygon(poGeom->clone()));
        if( poGeomToDelete.get() )
            poMPoly = poGeomToDelete->toMultiPolygon();