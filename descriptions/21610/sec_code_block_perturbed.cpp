const auto storeArcByCenterPointParameters = [](
        const CPLXMLNode* psChild,
        const char* l_pszSRSName,
        bool &bIsApproximateArc,
        double &dfLastCurveApproximateArcRadius,
        bool &bLastCurveWasApproximateArcInvertedAxisOrder)
    {
        const CPLXMLNode* psRadius =
            FindBareXMLChild(psChild, "radius");
        if( psRadius && psRadius->eType == CXT_Element )
        {
            double radius = CPLAtof(CPLGetXMLValue(
                psRadius, nullptr, "0"));
            const char* pszUnits = CPLGetXMLValue(
                psRadius, "uom", nullptr);
            bool bSRSUnitIsDegree = false;
            bool bInvertedAxisOrder = false;
            if( l_pszSRSName != nullptr )
            {
                OGRSpatialReference oSRS;
                if( oSRS.SetFromUserInput(l_pszSRSName)
                    == OGRERR_NONE )
                {
                    if( oSRS.IsGeographic() )
                    {
                        bInvertedAxisOrder =
                            CPL_TO_BOOL(oSRS.EPSGTreatsAsLatLong());
                        bSRSUnitIsDegree =
                            fabs(oSRS.GetAngularUnits(nullptr) -
                                    CPLAtof(SRS_UA_DEGREE_CONV))
                            < 1e-8;
                    }
                }
            }
            if( bSRSUnitIsDegree && pszUnits != nullptr &&
                (EQUAL(pszUnits, "m") ||
                 EQUAL(pszUnits, "nm") ||
                 EQUAL(pszUnits, "[nmi_i]") ||
                 EQUAL(pszUnits, "mi") ||
                 EQUAL(pszUnits, "ft")) )
            {
                bIsApproximateArc = true;
                if( EQUAL(pszUnits, "nm") || EQUAL(pszUnits, "[nmi_i]"))
                    radius *= CPLAtof(SRS_UL_INTL_NAUT_MILE_CONV);
                else if( EQUAL(pszUnits, "mi") )
                    radius *= CPLAtof(SRS_UL_INTL_STAT_MILE_CONV);
                else if( EQUAL(pszUnits, "ft") )
                    radius *= CPLAtof(SRS_UL_INTL_FOOT_CONV);
                dfLastCurveApproximateArcRadius = radius;
                bLastCurveWasApproximateArcInvertedAxisOrder =
                    bInvertedAxisOrder;
            }
        }
    };

    const auto connectArcByCenterPointToOtherSegments = [](
        OGRGeometry* poGeom,
        OGRCompoundCurve* poCC,
        const bool bIsApproximateArc,
        const bool bLastCurveWasApproximateArc,
        const double dfLastCurveApproximateArcRadius,
        const bool bLastCurveWasApproximateArcInvertedAxisOrder)
    {
        if( bIsApproximateArc )
        {
            if( poGeom->getGeometryType() == wkbLineString )
            {
                OGRCurve* poPreviousCurve =
                    poCC->getCurve(poCC->getNumCurves()-1);
                OGRLineString* poLS = poGeom->toLineString();
                if( poPreviousCurve->getNumPoints() >= 2 &&
                    poLS->getNumPoints() >= 2 )
                {
                    OGRPoint p;
                    OGRPoint p2;
                    poPreviousCurve->EndPoint(&p);
                    poLS->StartPoint(&p2);
                    double dfDistance = 0.0;
                    if( bLastCurveWasApproximateArcInvertedAxisOrder )
                        dfDistance =
                            OGR_GreatCircle_Distance(
                                p.getX(), p.getY(),
                                p2.getX(), p2.getY());
                    else
                        dfDistance =
                            OGR_GreatCircle_Distance(
                                p.getY(), p.getX(),
                                p2.getY(), p2.getX());
                    // CPLDebug("OGR", "%f %f",
                    //          dfDistance,
                    //          dfLastCurveApproximateArcRadius
                    //          / 10.0 );
                    if( dfDistance <
                        dfLastCurveApproximateArcRadius / 5.0 )
                    {
                        CPLDebug("OGR",
                                    "Moving approximate start of "
                                    "ArcByCenterPoint to end of "
                                    "previous curve");
                        poLS->setPoint(0, &p);
                    }
                }
            }
        }
        else if( bLastCurveWasApproximateArc )
        {
            OGRCurve* poPreviousCurve =
                poCC->getCurve(poCC->getNumCurves()-1);
            if( poPreviousCurve->getGeometryType() ==
                wkbLineString )
            {
                OGRLineString* poLS = poPreviousCurve->toLineString();
                OGRCurve *poAsCurve = poGeom->toCurve();

                if( poLS->getNumPoints() >= 2 &&
                    poAsCurve->getNumPoints() >= 2 )
                {
                    OGRPoint p;
                    OGRPoint p2;
                    poAsCurve->StartPoint(&p);
                    poLS->EndPoint(&p2);
                    double dfDistance = 0.0;
                    if( bLastCurveWasApproximateArcInvertedAxisOrder )
                        dfDistance =
                            OGR_GreatCircle_Distance(
                                p.getX(), p.getY(),
                                p2.getX(), p2.getY());
                    else
                        dfDistance =
                            OGR_GreatCircle_Distance(
                                p.getY(), p.getX(),
                                p2.getY(), p2.getX());
                    // CPLDebug(
                    //    "OGR", "%f %f",
                    //    dfDistance,
                    //    dfLastCurveApproximateArcRadius / 10.0 );

                    // "A-311 WHEELER AFB OAHU, HI.xml" needs more
                    // than 10%.
                    if( dfDistance <
                        dfLastCurveApproximateArcRadius / 5.0 )
                    {
                        CPLDebug(
                            "OGR",
                            "Moving approximate end of last "
                            "ArcByCenterPoint to start of the "
                            "current curve");
                        poLS->setPoint(poLS->getNumPoints() - 1,
                                        &p);
                    }
                }
            }
        }
    };

/* -------------------------------------------------------------------- */
/*      Ring GML3                                                       */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "Ring") )
    {
        OGRCurve* poRing = nullptr;
        OGRCompoundCurve *poCC = nullptr;
        bool bChildrenAreAllLineString = true;

        bool bLastCurveWasApproximateArc = false;
        bool bLastCurveWasApproximateArcInvertedAxisOrder = false;
        double dfLastCurveApproximateArcRadius = 0.0;

        bool bIsFirstChild = true;
        bool bFirstChildIsApproximateArc = false;
        double dfFirstChildApproximateArcRadius = 0.0;
        bool bFirstChildWasApproximateArcInvertedAxisOrder = false;

        for( const CPLXMLNode *psChild = psNode->psChild;
             psChild != nullptr;
             psChild = psChild->psNext )
        {
            if( psChild->eType == CXT_Element
                && EQUAL(BareGMLElement(psChild->pszValue), "curveMember") )
            {
                const CPLXMLNode* psCurveChild = GetChildElement(psChild);
                OGRGeometry* poGeom = nullptr;
                if( psCurveChild != nullptr )
                {
                    poGeom =
                        GML2OGRGeometry_XMLNode_Internal(
                            psCurveChild,
                            nPseudoBoolGetSecondaryGeometryOption,
                            nRecLevel + 1,
                            nSRSDimension,
                            pszSRSName );
                }
                else
                {
                    if( psChild->psChild && psChild->psChild->eType ==
                        CXT_Attribute &&
                        psChild->psChild->psNext == nullptr &&
                        strcmp(psChild->psChild->pszValue, "xlink:href") == 0 )
                    {
                        CPLError(CE_Warning, CPLE_AppDefined,
                                 "Cannot resolve xlink:href='%s'. "
                                 "Try setting GML_SKIP_RESOLVE_ELEMS=NONE",
                                 psChild->psChild->psChild->pszValue);
                    }
                    delete poRing;
                    delete poCC;
                    delete poGeom;
                    return nullptr;
                }

                // Try to join multiline string to one linestring.
                if( poGeom && wkbFlatten(poGeom->getGeometryType()) ==
                    wkbMultiLineString )
                {
                    poGeom =
                        OGRGeometryFactory::forceToLineString(poGeom, false);
                }

                if( poGeom == nullptr
                    || !OGR_GT_IsCurve(poGeom->getGeometryType()) )
                {
                    delete poGeom;
                    delete poRing;
                    delete poCC;
                    return nullptr;
                }

                if( wkbFlatten(poGeom->getGeometryType()) != wkbLineString )
                    bChildrenAreAllLineString = false;

                // Ad-hoc logic to handle nicely connecting ArcByCenterPoint
                // with consecutive curves, as found in some AIXM files.
                bool bIsApproximateArc = false;
                const CPLXMLNode* psChild2, *psChild3;
                if( strcmp(BareGMLElement(psCurveChild->pszValue), "Curve") == 0 &&
                    (psChild2 = GetChildElement(psCurveChild)) != nullptr &&
                    strcmp(BareGMLElement(psChild2->pszValue), "segments") == 0 &&
                    (psChild3 = GetChildElement(psChild2)) != nullptr &&
                    strcmp(BareGMLElement(psChild3->pszValue), "ArcByCenterPoint") == 0 )
                {
                    storeArcByCenterPointParameters(psChild3,
                                               pszSRSName,
                                               bIsApproximateArc,
                                               dfLastCurveApproximateArcRadius,
                                               bLastCurveWasApproximateArcInvertedAxisOrder);
                    if( bIsFirstChild && bIsApproximateArc )
                    {
                        bFirstChildIsApproximateArc = true;
                        dfFirstChildApproximateArcRadius = dfLastCurveApproximateArcRadius;
                        bFirstChildWasApproximateArcInvertedAxisOrder = bLastCurveWasApproximateArcInvertedAxisOrder;
                    }
                    else if( psChild3->psNext )
                    {
                        bIsApproximateArc = false;
                    }
                }
                bIsFirstChild = false;

                if( poCC == nullptr && poRing == nullptr )
                {
                    poRing = poGeom->toCurve();
                }
                else
                {
                    if( poCC == nullptr )
                    {
                        poCC = new OGRCompoundCurve();
                        bool bIgnored = false;
                        if( !GML2OGRGeometry_AddToCompositeCurve(poCC, poRing, bIgnored) )
                        {
                            delete poGeom;
                            delete poRing;
                            delete poCC;
                            return nullptr;
                        }
                        poRing = nullptr;
                    }

                    connectArcByCenterPointToOtherSegments(poGeom, poCC,
                                                           bIsApproximateArc,
                                                           bLastCurveWasApproximateArc,
                                                           dfLastCurveApproximateArcRadius,
                                                           bLastCurveWasApproximateArcInvertedAxisOrder);

                    OGRCurve *poCurve = poGeom->toCurve();

                    bool bIgnored = false;
                    if( !GML2OGRGeometry_AddToCompositeCurve( poCC,
                                                              poCurve,
                                                              bIgnored ) )
                    {
                        delete poGeom;
                        delete poCC;
                        delete poRing;
                        return nullptr;
                    }
                }

                bLastCurveWasApproximateArc = bIsApproximateArc;
            }
        }

        if( poRing )
        {
            if( poRing->getNumPoints() >= 2 &&
                bFirstChildIsApproximateArc && !poRing->get_IsClosed() &&
                wkbFlatten(poRing->getGeometryType()) == wkbLineString )
            {
                OGRLineString* poLS = poRing->toLineString();

                OGRPoint p;
                OGRPoint p2;
                poLS->StartPoint(&p);
                poLS->EndPoint(&p2);
                double dfDistance = 0.0;
                if( bFirstChildWasApproximateArcInvertedAxisOrder )
                    dfDistance =
                        OGR_GreatCircle_Distance(
                            p.getX(), p.getY(),
                            p2.getX(), p2.getY());
                else
                    dfDistance =
                        OGR_GreatCircle_Distance(
                            p.getY(), p.getX(),
                            p2.getY(), p2.getX());
                if( dfDistance <
                    dfFirstChildApproximateArcRadius / 5.0 )
                {
                    CPLDebug("OGR",
                             "Moving approximate start of "
                             "ArcByCenterPoint to end of "
                             "curve");
                    poLS->setPoint(0, &p2);
                }
            }

            if( poRing->getNumPoints() < 2 || !poRing->get_IsClosed() )
            {
                CPLError(CE_Failure, CPLE_AppDefined, "Non-closed ring");
                delete poRing;
                return nullptr;
            }
            return poRing;
        }

        if( poCC == nullptr )
            return nullptr;

        else if( bCastToLinearTypeIfPossible && bChildrenAreAllLineString )
        {
            return OGRCurve::CastToLinearRing(poCC);
        }
        else
        {
            if( poCC->getNumPoints() < 2 || !poCC->get_IsClosed() )
            {
                CPLError(CE_Failure, CPLE_AppDefined, "Non-closed ring");
                delete poCC;
                return nullptr;
            }
            return poCC;
        }
    }

/* -------------------------------------------------------------------- */
/*      LineString                                                      */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "LineString")
        || EQUAL(pszBaseGeometry, "LineStringSegment")
        || EQUAL(pszBaseGeometry, "GeodesicString") )
    {
        OGRLineString *poLine = new OGRLineString();

        if( !ParseGMLCoordinates( psNode, poLine, nSRSDimension ) )
        {
            delete poLine;
            return nullptr;
        }

        return poLine;
    }

/* -------------------------------------------------------------------- */
/*      Arc                                                             */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "Arc") )
    {
        OGRCircularString *poCC = new OGRCircularString();

        if( !ParseGMLCoordinates( psNode, poCC, nSRSDimension ) )
        {
            delete poCC;
            return nullptr;
        }

        // Normally a gml:Arc has only 3 points of controls, but in the
        // wild we sometimes find GML with 5 points, so accept any odd
        // number >= 3 (ArcString should be used for > 3 points)
        if( poCC->getNumPoints() < 3 || (poCC->getNumPoints() % 2) != 1 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Bad number of points in Arc");
            delete poCC;
            return nullptr;
        }

        return poCC;
    }

/* -------------------------------------------------------------------- */
/*     ArcString                                                        */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "ArcString") )
    {
        OGRCircularString *poCC = new OGRCircularString();

        if( !ParseGMLCoordinates( psNode, poCC, nSRSDimension ) )
        {
            delete poCC;
            return nullptr;
        }

        if( poCC->getNumPoints() < 3 || (poCC->getNumPoints() % 2) != 1 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Bad number of points in ArcString");
            delete poCC;
            return nullptr;
        }

        return poCC;
    }

/* -------------------------------------------------------------------- */
/*      Circle                                                          */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "Circle") )
    {
        OGRLineString *poLine = new OGRLineString();

        if( !ParseGMLCoordinates( psNode, poLine, nSRSDimension ) )
        {
            delete poLine;
            return nullptr;
        }

        if( poLine->getNumPoints() != 3 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Bad number of points in Circle");
            delete poLine;
            return nullptr;
        }

        double R = 0.0;
        double cx = 0.0;
        double cy = 0.0;
        double alpha0 = 0.0;
        double alpha1 = 0.0;
        double alpha2 = 0.0;
        if( !OGRGeometryFactory::GetCurveParmeters(
                               poLine->getX(0), poLine->getY(0),
                               poLine->getX(1), poLine->getY(1),
                               poLine->getX(2), poLine->getY(2),
                               R, cx, cy, alpha0, alpha1, alpha2 ) )
        {
            delete poLine;
            return nullptr;
        }

        OGRCircularString *poCC = new OGRCircularString();
        OGRPoint p;
        poLine->getPoint(0, &p);
        poCC->addPoint(&p);
        poLine->getPoint(1, &p);
        poCC->addPoint(&p);
        poLine->getPoint(2, &p);
        poCC->addPoint(&p);
        const double alpha4 =
            alpha2 > alpha0 ? alpha0 + kdf2PI : alpha0 - kdf2PI;
        const double alpha3 = (alpha2 + alpha4) / 2.0;
        const double x = cx + R * cos(alpha3);
        const double y = cy + R * sin(alpha3);
        if( poCC->getCoordinateDimension() == 3 )
            poCC->addPoint( x, y, p.getZ() );
        else
            poCC->addPoint( x, y );
        poLine->getPoint(0, &p);
        poCC->addPoint(&p);
        delete poLine;
        return poCC;
    }

/* -------------------------------------------------------------------- */
/*      ArcByBulge                                                      */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "ArcByBulge") )
    {
        const CPLXMLNode *psChild = FindBareXMLChild( psNode, "bulge");
        if( psChild == nullptr || psChild->eType != CXT_Element ||
            psChild->psChild == nullptr )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Missing bulge element." );
            return nullptr;
        }
        const double dfBulge = CPLAtof(psChild->psChild->pszValue);

        psChild = FindBareXMLChild( psNode, "normal");
        if( psChild == nullptr || psChild->eType != CXT_Element )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Missing normal element." );
            return nullptr;
        }
        double dfNormal = CPLAtof(psChild->psChild->pszValue);

        OGRLineString* poLS = new OGRLineString();
        if( !ParseGMLCoordinates( psNode, poLS, nSRSDimension ) )
        {
            delete poLS;
            return nullptr;
        }

        if( poLS->getNumPoints() != 2 )
        {
            CPLError(CE_Failure, CPLE_AppDefined,
                     "Bad number of points in ArcByBulge");
            delete poLS;
            return nullptr;
        }

        OGRCircularString *poCC = new OGRCircularString();
        OGRPoint p;
        poLS->getPoint(0, &p);
        poCC->addPoint(&p);

        const double dfMidX = (poLS->getX(0) + poLS->getX(1)) / 2.0;
        const double dfMidY = (poLS->getY(0) + poLS->getY(1)) / 2.0;
        const double dfDirX = (poLS->getX(1) - poLS->getX(0)) / 2.0;
        const double dfDirY = (poLS->getY(1) - poLS->getY(0)) / 2.0;
        double dfNormX = -dfDirY;
        double dfNormY = dfDirX;
        const double dfNorm = sqrt(dfNormX * dfNormX + dfNormY * dfNormY);
        if( dfNorm != 0.0 )
        {
            dfNormX /= dfNorm;
            dfNormY /= dfNorm;
        }
        const double dfNewX = dfMidX + dfNormX * dfBulge * dfNormal;
        const double dfNewY = dfMidY + dfNormY * dfBulge * dfNormal;

        if( poCC->getCoordinateDimension() == 3 )
            poCC->addPoint( dfNewX, dfNewY, p.getZ() );
        else
            poCC->addPoint( dfNewX, dfNewY );

        poLS->getPoint(1, &p);
        poCC->addPoint(&p);

        delete poLS;
        return poCC;
    }

/* -------------------------------------------------------------------- */
/*      ArcByCenterPoint                                                */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "ArcByCenterPoint") )
    {
        const CPLXMLNode *psChild = FindBareXMLChild( psNode, "radius");
        if( psChild == nullptr || psChild->eType != CXT_Element )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Missing radius element." );
            return nullptr;
        }
        const double radius =
            CPLAtof(CPLGetXMLValue(psChild,
                                   nullptr, "0"));
        const char* pszUnits =
            CPLGetXMLValue(psChild, "uom", nullptr);

        psChild = FindBareXMLChild( psNode, "startAngle");
        if( psChild == nullptr || psChild->eType != CXT_Element )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Missing startAngle element." );
            return nullptr;
        }
        const double dfStartAngle =
            CPLAtof(CPLGetXMLValue(psChild,
                                   nullptr, "0"));

        psChild = FindBareXMLChild( psNode, "endAngle");
        if( psChild == nullptr || psChild->eType != CXT_Element )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Missing endAngle element." );
            return nullptr;
        }
        const double dfEndAngle =
            CPLAtof(CPLGetXMLValue(psChild,
                                   nullptr, "0"));

        OGRPoint p;
        if( !ParseGMLCoordinates( psNode, &p, nSRSDimension ) )
        {
            return nullptr;
        }

        bool bSRSUnitIsDegree = false;
        bool bInvertedAxisOrder = false;
        if( pszSRSName != nullptr )
        {
            OGRSpatialReference oSRS;
            if( oSRS.SetFromUserInput(pszSRSName) == OGRERR_NONE )
            {
                if( oSRS.IsGeographic() )
                {
                    bInvertedAxisOrder =
                        CPL_TO_BOOL(oSRS.EPSGTreatsAsLatLong());
                    bSRSUnitIsDegree = fabs(oSRS.GetAngularUnits(nullptr) -
                                            CPLAtof(SRS_UA_DEGREE_CONV)) < 1e-8;
                }
            }
        }

        const double dfCenterX = p.getX();
        const double dfCenterY = p.getY();

        if( bSRSUnitIsDegree && pszUnits != nullptr &&
            (EQUAL(pszUnits, "m") ||
             EQUAL(pszUnits, "nm") ||
             EQUAL(pszUnits, "[nmi_i]") ||
             EQUAL(pszUnits, "mi") ||
             EQUAL(pszUnits, "ft")) )
        {
            OGRLineString* poLS = new OGRLineString();
            // coverity[tainted_data]
            const double dfStep =
                CPLAtof(CPLGetConfigOption("OGR_ARC_STEPSIZE", "4"));
            double dfDistance = radius;
            if( EQUAL(pszUnits, "nm") || EQUAL(pszUnits, "[nmi_i]") )
                dfDistance *= CPLAtof(SRS_UL_INTL_NAUT_MILE_CONV);
            else if( EQUAL(pszUnits, "mi") )
                dfDistance *= CPLAtof(SRS_UL_INTL_STAT_MILE_CONV);
            else if( EQUAL(pszUnits, "ft") )
                dfDistance *= CPLAtof(SRS_UL_INTL_FOOT_CONV);
            const double dfSign = dfStartAngle < dfEndAngle ? 1 : -1;
            for( double dfAngle = dfStartAngle;
                 (dfAngle - dfEndAngle) * dfSign < 0;
                 dfAngle += dfSign * dfStep)
            {
                double dfLong = 0.0;
                double dfLat = 0.0;
                if( bInvertedAxisOrder )
                {
                    OGR_GreatCircle_ExtendPosition(
                       dfCenterX, dfCenterY,
                       dfDistance,
                       // See https://ext.eurocontrol.int/aixm_confluence/display/ACG/ArcByCenterPoint+Interpretation+Summary
                       dfAngle,
                       &dfLat, &dfLong);
                    p.setX( dfLat ); // yes, external code will do the swap later
                    p.setY( dfLong );
                }
                else
                {
                    OGR_GreatCircle_ExtendPosition(dfCenterY, dfCenterX,
                                             dfDistance, 90-dfAngle,
                                             &dfLat, &dfLong);
                    p.setX( dfLong );
                    p.setY( dfLat );
                }
                poLS->addPoint(&p);
            }

            double dfLong = 0.0;
            double dfLat = 0.0;
            if( bInvertedAxisOrder )
            {
                OGR_GreatCircle_ExtendPosition(dfCenterX, dfCenterY,
                                         dfDistance,
                                         dfEndAngle,
                                         &dfLat, &dfLong);
                p.setX( dfLat ); // yes, external code will do the swap later
                p.setY( dfLong );
            }
            else
            {
                OGR_GreatCircle_ExtendPosition(dfCenterY, dfCenterX,
                                         dfDistance, 90-dfEndAngle,
                                         &dfLat, &dfLong);
                p.setX( dfLong );
                p.setY( dfLat );
            }
            poLS->addPoint(&p);

            return poLS;
        }

        OGRCircularString *poCC = new OGRCircularString();
        p.setX(dfCenterX + radius * cos(dfStartAngle * kdfD2R));
        p.setY(dfCenterY + radius * sin(dfStartAngle * kdfD2R));
        poCC->addPoint(&p);
        const double dfAverageAngle = (dfStartAngle + dfEndAngle) / 2.0;
        p.setX(dfCenterX + radius * cos(dfAverageAngle * kdfD2R));
        p.setY(dfCenterY + radius * sin(dfAverageAngle * kdfD2R));
        poCC->addPoint(&p);
        p.setX(dfCenterX + radius * cos(dfEndAngle * kdfD2R));
        p.setY(dfCenterY + radius * sin(dfEndAngle * kdfD2R));
        poCC->addPoint(&p);
        return poCC;
    }

/* -------------------------------------------------------------------- */
/*      CircleByCenterPoint                                             */
/* -------------------------------------------------------------------- */
    if( EQUAL(pszBaseGeometry, "CircleByCenterPoint") )
    {
        const CPLXMLNode *psChild = FindBareXMLChild( psNode, "radius");
        if( psChild == nullptr || psChild->eType != CXT_Element )
        {
            CPLError( CE_Failure, CPLE_AppDefined,
                      "Missing radius element." );
            return nullptr;
        }
        const double radius =
            CPLAtof(CPLGetXMLValue(psChild,
                                   nullptr, "0"));
        const char* pszUnits =
            CPLGetXMLValue(psChild, "uom", nullptr);

        OGRPoint p;
        if( !ParseGMLCoordinates( psNode, &p, nSRSDimension ) )
        {
            return nullptr;
        }

        bool bSRSUnitIsDegree = false;
        bool bInvertedAxisOrder = false;
        if( pszSRSName != nullptr )
        {
            OGRSpatialReference oSRS;
            if( oSRS.SetFromUserInput(pszSRSName) == OGRERR_NONE )
            {
                if( oSRS.IsGeographic() )
                {
                    bInvertedAxisOrder =
                        CPL_TO_BOOL(oSRS.EPSGTreatsAsLatLong());
                    bSRSUnitIsDegree = fabs(oSRS.GetAngularUnits(nullptr) -
                                            CPLAtof(SRS_UA_DEGREE_CONV)) < 1e-8;
                }
            }
        }

        const double dfCenterX = p.getX();
        const double dfCenterY = p.getY();

        if( bSRSUnitIsDegree && pszUnits != nullptr &&
            (EQUAL(pszUnits, "m") ||
             EQUAL(pszUnits, "nm") ||
             EQUAL(pszUnits, "[nmi_i]") ||
             EQUAL(pszUnits, "mi") ||
             EQUAL(pszUnits, "ft")) )
        {
            OGRLineString* poLS = new OGRLineString();
            const double dfStep =
                CPLAtof(CPLGetConfigOption("OGR_ARC_STEPSIZE", "4"));
            double dfDistance = radius;
            if( EQUAL(pszUnits, "nm") || EQUAL(pszUnits, "[nmi_i]") )
                dfDistance *= CPLAtof(SRS_UL_INTL_NAUT_MILE_CONV);
            else if( EQUAL(pszUnits, "mi") )
                dfDistance *= CPLAtof(SRS_UL_INTL_STAT_MILE_CONV);
            else if( EQUAL(pszUnits, "ft") )
                dfDistance *= CPLAtof(SRS_UL_INTL_FOOT_CONV);
            for( double dfAngle = 0; dfAngle < 360; dfAngle += dfStep )
            {
                double dfLong = 0.0;
                double dfLat = 0.0;
                if( bInvertedAxisOrder )
                {
                    OGR_GreatCircle_ExtendPosition(dfCenterX, dfCenterY,
                                             dfDistance, dfAngle,
                                             &dfLat, &dfLong);
                    p.setX( dfLat ); // yes, external code will do the swap later
                    p.setY( dfLong );
                }
                else
                {
                    OGR_GreatCircle_ExtendPosition(dfCenterY, dfCenterX,
                                             dfDistance, dfAngle,
                                             &dfLat, &dfLong);
                    p.setX( dfLong );
                    p.setY( dfLat );
                }
                poLS->addPoint(&p);
            }
            poLS->getPoint(0, &p);
            poLS->addPoint(&p);
            return poLS;
        }

        OGRCircularString *poCC = new OGRCircularString();
        p.setX( dfCenterX - radius );
        p.setY( dfCenterY );
        poCC->addPoint(&p);
        p.setX( dfCenterX + radius);
        p.setY( dfCenterY );
        poCC->addPoint(&p);
        p.setX( dfCenterX - radius );
        p.setY( dfCenterY );
        poCC->addPoint(&p);
        return poCC;
    }