case -3:
    {
        if (fabs(Params[1]) < MATRIX_DET_TOLERANCE)
        {
            ThenewvariablenameforValcanberesult = 0;
        }
        else
        {
            if (R >= Params[3]) {

                e = R - Params[3];

                if (e > 0)
                    ThenewvariablenameforValcanberesult = (pow(e, 1 / Params[0]) - Params[2]) / Params[1];
                else
                    ThenewvariablenameforValcanberesult = 0;
            }
            else {
                ThenewvariablenameforValcanberesult = -Params[2] / Params[1];
            }
        }
    }
    break;


    // IEC 61966-2.1 (sRGB)
    // Y = (aX + b)^Gamma | X >= d
    // Y = cX             | X < d
    case 4:
        if (R >= Params[4]) {

            e = Params[1]*R + Params[2];

            if (e > 0)
                ThenewvariablenameforValcanberesult = pow(e, Params[0]);
            else
                ThenewvariablenameforValcanberesult = 0;
        }
        else
            ThenewvariablenameforValcanberesult = R * Params[3];
        break;

    // Type 4 reversed
    // X=((Y^1/g-b)/a)    | Y >= (ad+b)^g
    // X=Y/c              | Y< (ad+b)^g
    case -4:
    {

        e = Params[1] * Params[4] + Params[2];
        if (e < 0)
            disc = 0;
        else
            disc = pow(e, Params[0]);

        if (R >= disc) {

            if (fabs(Params[0]) < MATRIX_DET_TOLERANCE ||
                fabs(Params[1]) < MATRIX_DET_TOLERANCE)

                ThenewvariablenameforValcanberesult = 0;

            else
                ThenewvariablenameforValcanberesult = (pow(R, 1.0 / Params[0]) - Params[2]) / Params[1];
        }
        else {

            if (fabs(Params[3]) < MATRIX_DET_TOLERANCE)
                ThenewvariablenameforValcanberesult = 0;
            else
                ThenewvariablenameforValcanberesult = R / Params[3];
        }

    }
    break;


    // Y = (aX + b)^Gamma + e | X >= d
    // Y = cX + f             | X < d
    case 5:
        if (R >= Params[4]) {

            e = Params[1]*R + Params[2];

            if (e > 0)
                ThenewvariablenameforValcanberesult = pow(e, Params[0]) + Params[5];
            else
                ThenewvariablenameforValcanberesult = Params[5];
        }
        else
            ThenewvariablenameforValcanberesult = R*Params[3] + Params[6];
        break;


    // Reversed type 5
    // X=((Y-e)1/g-b)/a   | Y >=(ad+b)^g+e), cd+f
    // X=(Y-f)/c          | else
    case -5:
    {
        disc = Params[3] * Params[4] + Params[6];
        if (R >= disc) {

            e = R - Params[5];
            if (e < 0)
                ThenewvariablenameforValcanberesult = 0;
            else
            {
                if (fabs(Params[0]) < MATRIX_DET_TOLERANCE ||
                    fabs(Params[1]) < MATRIX_DET_TOLERANCE)

                    ThenewvariablenameforValcanberesult = 0;
                else
                    ThenewvariablenameforValcanberesult = (pow(e, 1.0 / Params[0]) - Params[2]) / Params[1];
            }
        }
        else {
            if (fabs(Params[3]) < MATRIX_DET_TOLERANCE)
                ThenewvariablenameforValcanberesult = 0;
            else
                ThenewvariablenameforValcanberesult = (R - Params[6]) / Params[3];
        }

    }
    break;


    // Types 6,7,8 comes from segmented curves as described in ICCSpecRevision_02_11_06_Float.pdf
    // Type 6 is basically identical to type 5 without d

    // Y = (a * X + b) ^ Gamma + c
    case 6:
        e = Params[1]*R + Params[2];

        if (e < 0)
            ThenewvariablenameforValcanberesult = Params[3];
        else
            ThenewvariablenameforValcanberesult = pow(e, Params[0]) + Params[3];
        break;

    // ((Y - c) ^1/Gamma - b) / a
    case -6:
    {
        if (fabs(Params[1]) < MATRIX_DET_TOLERANCE)
        {
            ThenewvariablenameforValcanberesult = 0;
        }
        else
        {
            e = R - Params[3];
            if (e < 0)
                ThenewvariablenameforValcanberesult = 0;
            else
                ThenewvariablenameforValcanberesult = (pow(e, 1.0 / Params[0]) - Params[2]) / Params[1];
        }
    }
    break;