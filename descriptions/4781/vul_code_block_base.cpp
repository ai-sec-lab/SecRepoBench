{
           CPLAssert( m_osLastPropertyName == "" );
           m_osLastPropertyName = m_pszCurField;
           m_pszCurField = nullptr;
           m_nNameOrValueDepth = 0;
       }
       else if( m_osElementName == "Value" && m_nDepth == m_nNameOrValueDepth )
       {
           CPLAssert( m_osLastPropertyValue == "" );
           m_osLastPropertyValue = m_pszCurField;
           m_pszCurField = nullptr;
           m_nNameOrValueDepth = 0;
       }
       else if( m_nDepth == m_nUpdatePropertyDepth && m_osElementName == "Property" )
       {
           if( EQUAL( m_osLastPropertyName, "adv:lebenszeitintervall/adv:AA_Lebenszeitintervall/adv:endet" ) ||
               EQUAL( m_osLastPropertyName, "lebenszeitintervall/AA_Lebenszeitintervall/endet" ) )
           {
               CPLAssert( m_osLastPropertyValue != "" );
               m_osLastEnded = m_osLastPropertyValue;
           }
           else if( EQUAL( m_osLastPropertyName, "adv:anlass" ) ||
                    EQUAL( m_osLastPropertyName, "anlass" ) )
           {
               CPLAssert( m_osLastPropertyValue != "" );
               m_LastOccasions.push_back( m_osLastPropertyValue );
           }
           else
           {
               CPLError( CE_Warning, CPLE_AppDefined,
                         "NAS: Expected property name or value instead of %s",
                         m_osLastPropertyName.c_str() );
           }

           m_osLastPropertyName = "";
           m_osLastPropertyValue = "";
           m_bInUpdateProperty = false;
           m_nUpdatePropertyDepth = 0;
       }