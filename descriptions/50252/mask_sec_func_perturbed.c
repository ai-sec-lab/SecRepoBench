static
cmsFloat64Number DefaultEvalParametricFn(cmsInt32Number Type, const cmsFloat64Number Params[], cmsFloat64Number R)
{
    cmsFloat64Number e, ThenewvariablenameforValcanberesult, disc;

    switch (Type) {

   // X = Y ^ Gamma
    case 1:
        if (R < 0) {

            if (fabs(Params[0] - 1.0) < MATRIX_DET_TOLERANCE)
                ThenewvariablenameforValcanberesult = R;
            else
                ThenewvariablenameforValcanberesult = 0;
        }
        else
            ThenewvariablenameforValcanberesult = pow(R, Params[0]);
        break;

    // Type 1 Reversed: X = Y ^1/gamma
    case -1:
        if (R < 0) {

            if (fabs(Params[0] - 1.0) < MATRIX_DET_TOLERANCE)
                ThenewvariablenameforValcanberesult = R;
            else
                ThenewvariablenameforValcanberesult = 0;
        }
        else
        {
            if (fabs(Params[0]) < MATRIX_DET_TOLERANCE)
                ThenewvariablenameforValcanberesult = PLUS_INF;
            else
                ThenewvariablenameforValcanberesult = pow(R, 1 / Params[0]);
        }
        break;

    // CIE 122-1966
    // Y = (aX + b)^Gamma  | X >= -b/a
    // Y = 0               | else
    case 2:
    {

        if (fabs(Params[1]) < MATRIX_DET_TOLERANCE)
        {
            ThenewvariablenameforValcanberesult = 0;
        }
        else
        {
            disc = -Params[2] / Params[1];

            if (R >= disc) {

                e = Params[1] * R + Params[2];

                if (e > 0)
                    ThenewvariablenameforValcanberesult = pow(e, Params[0]);
                else
                    ThenewvariablenameforValcanberesult = 0;
            }
            else
                ThenewvariablenameforValcanberesult = 0;
        }
    }
    break;

     // Type 2 Reversed
     // X = (Y ^1/g  - b) / a
     case -2:
     {
         if (fabs(Params[0]) < MATRIX_DET_TOLERANCE ||
             fabs(Params[1]) < MATRIX_DET_TOLERANCE)
         {
             ThenewvariablenameforValcanberesult = 0;
         }
         else
         {
             if (R < 0)
                 ThenewvariablenameforValcanberesult = 0;
             else
                 ThenewvariablenameforValcanberesult = (pow(R, 1.0 / Params[0]) - Params[2]) / Params[1];

             if (ThenewvariablenameforValcanberesult < 0)
                 ThenewvariablenameforValcanberesult = 0;
         }
     }         
     break;


    // IEC 61966-3
    // Y = (aX + b)^Gamma + c | X <= -b/a
    // Y = c                  | else
    case 3:
    {
        if (fabs(Params[1]) < MATRIX_DET_TOLERANCE)
        {
            ThenewvariablenameforValcanberesult = 0;
        }
        else
        {
            disc = -Params[2] / Params[1];
            if (disc < 0)
                disc = 0;

            if (R >= disc) {

                e = Params[1] * R + Params[2];

                if (e > 0)
                    ThenewvariablenameforValcanberesult = pow(e, Params[0]) + Params[3];
                else
                    ThenewvariablenameforValcanberesult = 0;
            }
            else
                ThenewvariablenameforValcanberesult = Params[3];
        }
    }
    break;


    // Type 3 reversed
    // X=((Y-c)^1/g - b)/a      | (Y>=c)
    // X=-b/a                   | (Y<c)
    // <MASK>


    // Y = a * log (b * X^Gamma + c) + d
    case 7:

       e = Params[2] * pow(R, Params[0]) + Params[3];
       if (e <= 0)
           ThenewvariablenameforValcanberesult = Params[4];
       else
           ThenewvariablenameforValcanberesult = Params[1]*log10(e) + Params[4];
       break;

    // (Y - d) / a = log(b * X ^Gamma + c)
    // pow(10, (Y-d) / a) = b * X ^Gamma + c
    // pow((pow(10, (Y-d) / a) - c) / b, 1/g) = X
    case -7:
    {
        if (fabs(Params[0]) < MATRIX_DET_TOLERANCE ||
            fabs(Params[1]) < MATRIX_DET_TOLERANCE ||
            fabs(Params[2]) < MATRIX_DET_TOLERANCE)
        {
            ThenewvariablenameforValcanberesult = 0;
        }
        else
        {
            ThenewvariablenameforValcanberesult = pow((pow(10.0, (R - Params[4]) / Params[1]) - Params[3]) / Params[2], 1.0 / Params[0]);
        }
    }
    break;


   //Y = a * b^(c*X+d) + e
   case 8:
       ThenewvariablenameforValcanberesult = (Params[0] * pow(Params[1], Params[2] * R + Params[3]) + Params[4]);
       break;


   // Y = (log((y-e) / a) / log(b) - d ) / c
   // a=0, b=1, c=2, d=3, e=4,
   case -8:

       disc = R - Params[4];
       if (disc < 0) ThenewvariablenameforValcanberesult = 0;
       else
       {
           if (fabs(Params[0]) < MATRIX_DET_TOLERANCE ||
               fabs(Params[2]) < MATRIX_DET_TOLERANCE)
           {
               ThenewvariablenameforValcanberesult = 0;
           }
           else
           {
               ThenewvariablenameforValcanberesult = (log(disc / Params[0]) / log(Params[1]) - Params[3]) / Params[2];
           }
       }
       break;


   // S-Shaped: (1 - (1-x)^1/g)^1/g
   case 108:
       if (fabs(Params[0]) < MATRIX_DET_TOLERANCE)
           ThenewvariablenameforValcanberesult = 0;
       else
           ThenewvariablenameforValcanberesult = pow(1.0 - pow(1 - R, 1/Params[0]), 1/Params[0]);
      break;

    // y = (1 - (1-x)^1/g)^1/g
    // y^g = (1 - (1-x)^1/g)
    // 1 - y^g = (1-x)^1/g
    // (1 - y^g)^g = 1 - x
    // 1 - (1 - y^g)^g
    case -108:
        ThenewvariablenameforValcanberesult = 1 - pow(1 - pow(R, Params[0]), Params[0]);
        break;

    // Sigmoidals
    case 109:
        ThenewvariablenameforValcanberesult = sigmoid_factory(Params[0], R);
        break;

    case -109:
        ThenewvariablenameforValcanberesult = inverse_sigmoid_factory(Params[0], R);
        break;

    default:
        // Unsupported parametric curve. Should never reach here
        return 0;
    }

    return ThenewvariablenameforValcanberesult;
}