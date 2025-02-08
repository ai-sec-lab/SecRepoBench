if (b0 == (256 | 12)) {
                ptpriv->type1data.StemSnapH.count = n;
                for (f = 0, i = 0; i < n; f += args[i].fval, i++)
                    ptpriv->type1data.StemSnapH.values[i] = f;
            }

            if (b0 == (256 | 13)) {
                ptpriv->type1data.StemSnapV.count = n;
                for (f = 0, i = 0; i < n; f += args[i].fval, i++)
                    ptpriv->type1data.StemSnapV.values[i] = f;
            }

            if (b0 == (256 | 14))
                ptpriv->type1data.ForceBold = args[0].ival;

            if (b0 == (256 | 17))
                ptpriv->type1data.LanguageGroup = args[0].ival;

            if (b0 == (256 | 18))
                ptpriv->type1data.ExpansionFactor = args[0].fval;

            n = 0;