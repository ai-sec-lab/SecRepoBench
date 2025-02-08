if (b0 == 6) {
                ptpriv->type1data.BlueValues.count = n;
                ptpriv->type1data.BlueValues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    ptpriv->type1data.BlueValues.values[i] = ptpriv->type1data.BlueValues.values[i - 1] + args[i].fval;
                }
            }

            if (b0 == 7) {
                ptpriv->type1data.OtherBlues.count = n;
                ptpriv->type1data.OtherBlues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    ptpriv->type1data.OtherBlues.values[i] = ptpriv->type1data.OtherBlues.values[i - 1] + args[i].fval;
                }
            }

            if (b0 == 8) {
                ptpriv->type1data.FamilyBlues.count = n;
                ptpriv->type1data.FamilyBlues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    ptpriv->type1data.FamilyBlues.values[i] = ptpriv->type1data.FamilyBlues.values[i - 1] + args[i].fval;
                }
            }

            if (b0 == 9) {
                ptpriv->type1data.FamilyOtherBlues.count = n;
                ptpriv->type1data.FamilyOtherBlues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    ptpriv->type1data.FamilyOtherBlues.values[i] = ptpriv->type1data.FamilyOtherBlues.values[i - 1] + args[i].fval;
                }
            }