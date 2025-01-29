poGeomToDelete = std::unique_ptr<OGRGeometry>(
                OGRGeometryFactory::forceToMultiPolygon(poGeom->clone()));
        if( poGeomToDelete.get() &&
            wkbFlatten(poGeomToDelete->getGeometryType()) == wkbMultiPolygon )
        {
            poMPoly = poGeomToDelete->toMultiPolygon();
        }