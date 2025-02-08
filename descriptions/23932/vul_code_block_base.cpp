SkShader::GradientInfo grInfo;
    grInfo.fColorCount = 0;
    if (SkShader::kLinear_GradientType != shader->asAGradient(&grInfo)) {
        // TODO: non-linear gradient support
        return;
    }