static
_cmsTRANSFORM* AllocEmptyTransform(cmsContext ContextID, cmsPipeline* lut,
                                               cmsUInt32Number Intent, cmsUInt32Number* InputFormat, cmsUInt32Number* OutputFormat, cmsUInt32Number* dwFlags)
{
     _cmsTransformPluginChunkType* ctx = ( _cmsTransformPluginChunkType*) _cmsContextGetClientChunk(ContextID, TransformPlugin);
     _cmsTransformCollection* Plugin;

       // Allocate needed memory
       _cmsTRANSFORM* p = (_cmsTRANSFORM*)_cmsMallocZero(ContextID, sizeof(_cmsTRANSFORM));
       if (!p) {
              cmsPipelineFree(lut);
              return NULL;
       }

       // Store the proposed pipeline
       p->Lut = lut;

       // Let's see if any plug-in want to do the transform by itself
       if (p->Lut != NULL) {

           if (!(*dwFlags & cmsFLAGS_NOOPTIMIZE))
           {
               for (Plugin = ctx->TransformCollection;
                   Plugin != NULL;
                   Plugin = Plugin->Next) {

                   if (Plugin->Factory(&p->xform, &p->UserData, &p->FreeUserData, &p->Lut, InputFormat, OutputFormat, dwFlags)) {

                       // Last plugin in the declaration order takes control. We just keep
                       // the original parameters as a logging. 
                       // Note that cmsFLAGS_CAN_CHANGE_FORMATTER is not set, so by default 
                       // an optimized transform is not reusable. The plug-in can, however, change
                       // the flags and make it suitable.

                       p->ContextID = ContextID;
                       p->InputFormat = *InputFormat;
                       p->OutputFormat = *OutputFormat;
                       p->dwOriginalFlags = *dwFlags;

                       // Fill the formatters just in case the optimized routine is interested.
                       // No error is thrown if the formatter doesn't exist. It is up to the optimization 
                       // factory to decide what to do in those cases.
                       p->FromInput = _cmsGetFormatter(ContextID, *InputFormat, cmsFormatterInput, CMS_PACK_FLAGS_16BITS).Fmt16;
                       p->ToOutput = _cmsGetFormatter(ContextID, *OutputFormat, cmsFormatterOutput, CMS_PACK_FLAGS_16BITS).Fmt16;
                       p->FromInputFloat = _cmsGetFormatter(ContextID, *InputFormat, cmsFormatterInput, CMS_PACK_FLAGS_FLOAT).FmtFloat;
                       p->ToOutputFloat = _cmsGetFormatter(ContextID, *OutputFormat, cmsFormatterOutput, CMS_PACK_FLAGS_FLOAT).FmtFloat;

                       // Save the day? (Ignore the warning)
                       if (Plugin->OldXform) {
                           p->OldXform = (_cmsTransformFn)(void*)p->xform;
                           p->xform = _cmsTransform2toTransformAdaptor;
                       }

                       ParalellizeIfSuitable(p);
                       return p;
                   }
               }
           }

           // Not suitable for the transform plug-in, let's check  the pipeline plug-in
           _cmsOptimizePipeline(ContextID, &p->Lut, Intent, InputFormat, OutputFormat, dwFlags);
       }

    // Check whatever this is a true floating point transform
    if (_cmsFormatterIsFloat(*OutputFormat)) {

        // Get formatter function always return a valid union, but the contents of this union may be NULL.
        p ->FromInputFloat = _cmsGetFormatter(ContextID, *InputFormat,  cmsFormatterInput, CMS_PACK_FLAGS_FLOAT).FmtFloat;
        p ->ToOutputFloat  = _cmsGetFormatter(ContextID, *OutputFormat, cmsFormatterOutput, CMS_PACK_FLAGS_FLOAT).FmtFloat;
        *dwFlags |= cmsFLAGS_CAN_CHANGE_FORMATTER;

        if (p ->FromInputFloat == NULL || p ->ToOutputFloat == NULL) {

            cmsSignalError(ContextID, cmsERROR_UNKNOWN_EXTENSION, "Unsupported raster format");
            cmsDeleteTransform(p);
            return NULL;
        }

        if (*dwFlags & cmsFLAGS_NULLTRANSFORM) {

            p ->xform = NullFloatXFORM;
        }
        else {
            // Float transforms don't use cache, always are non-NULL
            p ->xform = FloatXFORM;
        }

    }
    else {

        // Formats are intended to be changed before use
        if // <MASK>

        if (*dwFlags & cmsFLAGS_NULLTRANSFORM) {

            p ->xform = NullXFORM;
        }
        else {
            if (*dwFlags & cmsFLAGS_NOCACHE) {

                if (*dwFlags & cmsFLAGS_GAMUTCHECK)
                    p ->xform = PrecalculatedXFORMGamutCheck;  // Gamut check, no cache
                else
                    p ->xform = PrecalculatedXFORM;  // No cache, no gamut check
            }
            else {

                if (*dwFlags & cmsFLAGS_GAMUTCHECK)
                    p ->xform = CachedXFORMGamutCheck;    // Gamut check, cache
                else
                    p ->xform = CachedXFORM;  // No gamut check, cache

            }
        }
    }

    p ->InputFormat     = *InputFormat;
    p ->OutputFormat    = *OutputFormat;
    p ->dwOriginalFlags = *dwFlags;
    p ->ContextID       = ContextID;
    p ->UserData        = NULL;
    ParalellizeIfSuitable(p);
    return p;
}