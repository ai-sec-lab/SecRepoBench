SkShader::GradientInfo gradientInfo;
    memset(&gradientInfo, 0, sizeof(gradientInfo));
    if (SkShader::kLinear_GradientType != shader->asAGradient(&gradientInfo)) {
        // TODO: non-linear gradient support
        return;
    }