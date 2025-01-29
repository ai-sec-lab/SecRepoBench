if (fabs(iir->biquads[current_biquad].b[0] +
                     iir->biquads[current_biquad].b[1] +
                     iir->biquads[current_biquad].b[2]) > 1e-6) {
                factor = (iir->biquads[current_biquad].a[0] +
                          iir->biquads[current_biquad].a[1] +
                          iir->biquads[current_biquad].a[2]) /
                         (iir->biquads[current_biquad].b[0] +
                          iir->biquads[current_biquad].b[1] +
                          iir->biquads[current_biquad].b[2]);

                av_log(ctx, AV_LOG_VERBOSE, "factor=%f\n", factor);

                iir->biquads[current_biquad].b[0] *= factor;
                iir->biquads[current_biquad].b[1] *= factor;
                iir->biquads[current_biquad].b[2] *= factor;
            }

            iir->biquads[current_biquad].b[0] *= (current_biquad ? 1.0 : iir->g);
            iir->biquads[current_biquad].b[1] *= (current_biquad ? 1.0 : iir->g);
            iir->biquads[current_biquad].b[2] *= (current_biquad ? 1.0 : iir->g);

            av_log(ctx, AV_LOG_VERBOSE, "a=%f %f %f:b=%f %f %f\n",
                   iir->biquads[current_biquad].a[0],
                   iir->biquads[current_biquad].a[1],
                   iir->biquads[current_biquad].a[2],
                   iir->biquads[current_biquad].b[0],
                   iir->biquads[current_biquad].b[1],
                   iir->biquads[current_biquad].b[2]);

            current_biquad++;