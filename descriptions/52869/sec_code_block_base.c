#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            /*
             * Avoid superlinear expansion by limiting the total number
             * of replacements.
             */
            if (ctxt->incTotal >= 20)
                break;
#endif
            ctxt->incTotal++;
            ref = xmlXIncludeExpandNode(ctxt, cur);
            /*
             * Mark direct includes.
             */
            if (ref != NULL)
                ref->replace = 1;