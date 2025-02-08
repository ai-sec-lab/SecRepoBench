SkShader::GradientInfo gradientInfo;
    gradientInfo.fColorCount = 0;
    if (SkShader::kLinear_GradientType != shader->asAGradient(&gradientInfo)) {
        // TODO: non-linear gradient support
        return;
    }