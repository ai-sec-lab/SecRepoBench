/*      additional vertex, so we need to create it ourselves.           */
/* -------------------------------------------------------------------- */

    if( bWantExtension && bHasTextAnnotation && nNumVertices >= 2 )
    {
        OGRPoint oLastVertex;
        poLine->getPoint( poLine->getNumPoints() - 1, &oLastVertex );

        double dfExtensionX = oLastVertex.getX();
        double dfExtensionY = oLastVertex.getY();
        double dfExtensionZ = oLastVertex.getZ();

        double dfExtensionLength = ( dfTextOffset * dfScale ) +
            dfTextAnnotationWidth;
        dfExtensionX += dfHorizontalDirectionX * dfExtensionLength;
        dfExtensionY += dfHorizontalDirY * dfExtensionLength;
        dfExtensionZ += dfHorizontalDirectionZ * dfExtensionLength;

        poLine->setPoint( poLine->getNumPoints(), dfExtensionX, dfExtensionY,
            dfExtensionZ );
    }