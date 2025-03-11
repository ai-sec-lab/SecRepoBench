#ifdef FUZZING_BUILD_MODE_UNSAFE_FOR_PRODUCTION
            /*
             * Avoid superlinear expansion by limiting the total number
             * of replacements.
             */
            if (include_context->incTotal >= 20)
                break;
#endif
            include_context->incTotal++;
            ref = xmlXIncludeExpandNode(include_context, cur);
            /*
             * Mark direct includes.
             */
            if (ref != NULL)
                ref->replace = 1;