if (Lab.L > 50) Lab.L = 50;
    if (Lab.L < 0) Lab.L = 0;

    // Free the resources
    cmsDeleteTransform(xform);

    // Convert from Lab (which is now clipped) to XYZ.
    cmsLab2XYZ(NULL, &BlackXYZ, &Lab);

    if (BlackPoint != NULL)
        *BlackPoint = BlackXYZ;

    return TRUE;

    cmsUNUSED_PARAMETER(dwFlags);