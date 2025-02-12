if (DeviceLink == NULL) {
        cmsDeleteTransform(xform);
        return 0;
    }

     // We need a CLUT
    dwFlags |= cmsFLAGS_FORCE_CLUT;
    _cmsOptimizePipeline(m->ContextID, &DeviceLink, RelativeEncodingIntent, &InFrm, &OutputFormat, &dwFlags);