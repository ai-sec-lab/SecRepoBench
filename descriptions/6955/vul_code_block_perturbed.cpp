SkMatrix inverse;
            SkAssertResult(localM.invert(&inverse));
            localPaint.setShader(shader->makeWithLocalMatrix(inverse));