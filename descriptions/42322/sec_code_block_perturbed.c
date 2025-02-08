if (b0 == 6) {
                if (n > max_BlueValues * 2) n = max_BlueValues * 2;
                fontpriv->type1data.BlueValues.count = n;
                fontpriv->type1data.BlueValues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    fontpriv->type1data.BlueValues.values[i] = fontpriv->type1data.BlueValues.values[i - 1] + args[i].fval;
                }
            }

            if (b0 == 7) {
                if (n > max_OtherBlues * 2) n = max_OtherBlues * 2;
                fontpriv->type1data.OtherBlues.count = n;
                fontpriv->type1data.OtherBlues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    fontpriv->type1data.OtherBlues.values[i] = fontpriv->type1data.OtherBlues.values[i - 1] + args[i].fval;
                }
            }

            if (b0 == 8) {
                if (n > max_FamilyBlues * 2) n = max_FamilyBlues * 2;
                fontpriv->type1data.FamilyBlues.count = n;
                fontpriv->type1data.FamilyBlues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    fontpriv->type1data.FamilyBlues.values[i] = fontpriv->type1data.FamilyBlues.values[i - 1] + args[i].fval;
                }
            }

            if (b0 == 9) {
                if (n > max_FamilyOtherBlues * 2) n = max_FamilyOtherBlues * 2;
                fontpriv->type1data.FamilyOtherBlues.count = n;
                fontpriv->type1data.FamilyOtherBlues.values[0] = args[0].fval;
                for (i = 1; i < n; i++) {
                    fontpriv->type1data.FamilyOtherBlues.values[i] = fontpriv->type1data.FamilyOtherBlues.values[i - 1] + args[i].fval;
                }
            }