static
cmsBool  BlackPointAsDarkerColorant(cmsHPROFILE    hInput,
                                    cmsUInt32Number Intent,
                                    cmsCIEXYZ* BlackPoint,
                                    cmsUInt32Number dwFlags)
{
    cmsUInt16Number *Black;
    cmsHTRANSFORM xform;
    cmsColorSpaceSignature Space;
    cmsUInt32Number nChannels;
    cmsUInt32Number dwFormat;
    cmsHPROFILE hLab;
    cmsCIELab  Lab;
    cmsCIEXYZ  BlackXYZ;
    cmsContext ContextID = cmsGetProfileContextID(hInput);

    // If the profile does not support input direction, assume Black point 0
    if (!cmsIsIntentSupported(hInput, Intent, LCMS_USED_AS_INPUT)) {

        BlackPoint -> X = BlackPoint ->Y = BlackPoint -> Z = 0.0;
        return FALSE;
    }

    // Create a formatter which has n channels and no floating point
    dwFormat = cmsFormatterForColorspaceOfProfile(hInput, 2, FALSE);

    // Try to get black by using black colorant
    Space = cmsGetColorSpace(hInput);

    // This function returns darker colorant in 16 bits for several spaces
    if (!_cmsEndPointsBySpace(Space, NULL, &Black, &nChannels)) {

        BlackPoint -> X = BlackPoint ->Y = BlackPoint -> Z = 0.0;
        return FALSE;
    }

    if (nChannels != T_CHANNELS(dwFormat)) {
       BlackPoint -> X = BlackPoint ->Y = BlackPoint -> Z = 0.0;
       return FALSE;
    }

    // Lab will be used as the output space, but lab2 will avoid recursion
    hLab = cmsCreateLab2ProfileTHR(ContextID, NULL);
    if (hLab == NULL) {
       BlackPoint -> X = BlackPoint ->Y = BlackPoint -> Z = 0.0;
       return FALSE;
    }

    // Create the transform
    xform = cmsCreateTransformTHR(ContextID, hInput, dwFormat,
                                hLab, TYPE_Lab_DBL, Intent, cmsFLAGS_NOOPTIMIZE|cmsFLAGS_NOCACHE);
    cmsCloseProfile(hLab);

    if (xform == NULL) {

        // Something went wrong. Get rid of open resources and return zero as black
        BlackPoint -> X = BlackPoint ->Y = BlackPoint -> Z = 0.0;
        return FALSE;
    }

    // Convert black to Lab
    cmsDoTransform(xform, Black, &Lab, 1);

    // Force it to be neutral, check for inconsistences
    Lab.a = Lab.b = 0;
    if (Lab.L > 50 || Lab.L < 0) Lab.L = 0;

    // Free the resources
    cmsDeleteTransform(xform);

    // Convert from Lab (which is now clipped) to XYZ.
    cmsLab2XYZ(NULL, &BlackXYZ, &Lab);

    if (BlackPoint != NULL)
        *BlackPoint = BlackXYZ;

    return TRUE;

    cmsUNUSED_PARAMETER(dwFlags);
}