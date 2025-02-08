SkMatrix inverse;
            if (localM.invert(&inverse)) {
                localPaint.setShader(shader->makeWithLocalMatrix(inverse));
            } else {
                localPaint.setShader(nullptr);  // can't handle this xform
            }