#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            /*
             * Avoid superlinear expansion by limiting the total number
             * of replacements.
             */
            if (ctxt->incTotal >= 20)
                return(-1);
#endif
            ctxt->incTotal++;
            ref = xmlXIncludeExpandNode(ctxt, cur);