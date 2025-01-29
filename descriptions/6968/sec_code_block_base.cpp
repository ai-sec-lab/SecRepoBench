std::vector< double > adfBulges = poCADLWPolyline->getBulges();
            const size_t nCount = std::min(adfBulges.size(), poCADLWPolyline->getVertexCount());

            for( size_t iCurrentVertex = 0; iCurrentVertex + 1 < nCount; iCurrentVertex++ )
            {
                CADVector stCurrentVertex = poCADLWPolyline->getVertex( iCurrentVertex );
                CADVector stNextVertex = poCADLWPolyline->getVertex( iCurrentVertex + 1 );

                double dfLength = sqrt( pow( stNextVertex.getX() - stCurrentVertex.getX(), 2 )
                                      + pow( stNextVertex.getY() - stCurrentVertex.getY(), 2 ) );

                /*
                 * Handling straight polyline segment.
                 */
                if( ( dfLength == 0 ) || ( adfBulges[iCurrentVertex] == 0 ) )
                {
                    if( !bLineStringStarted )
                    {
                        poLS->addPoint( stCurrentVertex.getX(),
                                        stCurrentVertex.getY(),
                                        stCurrentVertex.getZ()
                        );
                        bLineStringStarted = true;
                    }

                    poLS->addPoint( stNextVertex.getX(),
                                    stNextVertex.getY(),
                                    stNextVertex.getZ()
                    );
                }
                else
                {
                    double dfSegmentBulge = adfBulges[iCurrentVertex];
                    double dfH = ( dfSegmentBulge * dfLength ) / 2;
                    if( dfH == 0.0 )
                        dfH = 1.0; // just to avoid a division by zero
                    double dfRadius = ( dfH / 2 ) + ( dfLength * dfLength / ( 8 * dfH ) );
                    double dfOgrArcRotation = 0, dfOgrArcRadius = fabs( dfRadius );

                    /*
                     * Set arc's direction and keep bulge positive.
                     */
                    bool   bClockwise = ( dfSegmentBulge < 0 );
                    if( bClockwise )
                        dfSegmentBulge *= -1;

                    /*
                     * Get arc's center point.
                     */
                    double dfSaggita = fabs( dfSegmentBulge * ( dfLength / 2.0 ) );
                    double dfApo = bClockwise ? -( dfOgrArcRadius - dfSaggita ) :
                                                -( dfSaggita - dfOgrArcRadius );

                    CADVector stVertex;
                    stVertex.setX( stCurrentVertex.getX() - stNextVertex.getX() );
                    stVertex.setY( stCurrentVertex.getY() - stNextVertex.getY() );
                    stVertex.setZ( stCurrentVertex.getZ() );

                    CADVector stMidPoint;
                    stMidPoint.setX( stNextVertex.getX() + 0.5 * stVertex.getX() );
                    stMidPoint.setY( stNextVertex.getY() + 0.5 * stVertex.getY() );
                    stMidPoint.setZ( stVertex.getZ() );

                    CADVector stPperp;
                    stPperp.setX( stVertex.getY() );
                    stPperp.setY( -stVertex.getX() );
                    double dfStPperpLength = sqrt( stPperp.getX() * stPperp.getX() +
                                                   stPperp.getY() * stPperp.getY() );
                    // TODO: Check that length isnot 0
                    stPperp.setX( stPperp.getX() / dfStPperpLength );
                    stPperp.setY( stPperp.getY() / dfStPperpLength );

                    CADVector stOgrArcCenter;
                    stOgrArcCenter.setX( stMidPoint.getX() + ( stPperp.getX() * dfApo ) );
                    stOgrArcCenter.setY( stMidPoint.getY() + ( stPperp.getY() * dfApo ) );

                    /*
                     * Get the line's general vertical direction ( -1 = down, +1 = up ).
                     */
                    double dfLineDir = stNextVertex.getY() >
                                            stCurrentVertex.getY() ? 1.0f : -1.0f;

                    /*
                     * Get arc's starting angle.
                     */
                    double dfA = atan2( ( stOgrArcCenter.getY() - stCurrentVertex.getY() ),
                                        ( stOgrArcCenter.getX() - stCurrentVertex.getX() ) ) * DEG2RAD;
                    if( bClockwise && ( dfLineDir == 1.0 ) )
                        dfA += ( dfLineDir * 180.0 );

                    double dfOgrArcStartAngle = dfA > 0.0 ? -( dfA - 180.0 ) :
                                                            -( dfA + 180.0 );

                    /*
                     * Get arc's ending angle.
                     */
                    dfA = atan2( ( stOgrArcCenter.getY() - stNextVertex.getY() ),
                                 ( stOgrArcCenter.getX() - stNextVertex.getX() ) ) * DEG2RAD;
                    if( bClockwise && ( dfLineDir == 1.0 ) )
                        dfA += ( dfLineDir * 180.0 );

                    double dfOgrArcEndAngle = dfA > 0.0 ? -( dfA - 180.0 ) :
                                                          -( dfA + 180.0 );

                    if( !bClockwise && ( dfOgrArcStartAngle < dfOgrArcEndAngle) )
                        dfOgrArcEndAngle = -180.0 + ( dfLineDir * dfA );

                    if( bClockwise && ( dfOgrArcStartAngle > dfOgrArcEndAngle ) )
                        dfOgrArcEndAngle += 360.0;

                    /*
                     * Flip arc's rotation if necessary.
                     */
                    if( bClockwise && ( dfLineDir == 1.0 ) )
                        dfOgrArcRotation = dfLineDir * 180.0;

                    /*
                     * Tesselate the arc segment and append to the linestring.
                     */
                    OGRLineString * poArcpoLS =
                        ( OGRLineString * ) OGRGeometryFactory::approximateArcAngles(
                            stOgrArcCenter.getX(), stOgrArcCenter.getY(), stOgrArcCenter.getZ(),
                            dfOgrArcRadius, dfOgrArcRadius, dfOgrArcRotation,
                            dfOgrArcStartAngle,dfOgrArcEndAngle,
                            0.0 );

                    poLS->addSubLineString( poArcpoLS );

                    delete( poArcpoLS );
                }
            }