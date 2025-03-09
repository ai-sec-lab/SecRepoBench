if (DeviceLink == NULL) {
        cmsDeleteTransform(xform);
        cmsSignalError(m->ContextID, cmsERROR_CORRUPTION_DETECTED, "Cannot access link for CRD");
        return 0;
    }

     // We need a CLUT
    dwFlags |= cmsFLAGS_FORCE_CLUT;
    if (!_cmsOptimizePipeline(m->ContextID, &DeviceLink, AdjustedEncodingIntent, &InFrm, &OutputFormat, &dwFlags)) {
        cmsDeleteTransform(xform);
        cmsSignalError(m->ContextID, cmsERROR_CORRUPTION_DETECTED, "Cannot create CLUT table for CRD");
        return 0;
    }