if( static_cast<int>(OSR_GDV(papszPrj, "zone", 0.0)) != 0 )
        {
            const double dfYShift = OSR_GDV(papszPrj, "Yshift", 0.0);

            SetUTM( static_cast<int>(OSR_GDV(papszPrj, "zone", 0.0)),
                    dfYShift == 0.0 );
        }
        else
        {
            const double dfCentralMeridian = OSR_GDV(papszPrj, "PARAM_1", 0.0);
            const double dfRefLat = OSR_GDV(papszPrj, "PARAM_2", 0.0);

            const int nZone = static_cast<int>(
                (dfCentralMeridian + 183.0) / 6.0 + 0.0000001 );
            SetUTM( nZone, dfRefLat >= 0.0 );
        }