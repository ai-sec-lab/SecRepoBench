SkShader::GradientInfo grInfo;
    memset(&grInfo, 0, sizeof(grInfo));
    if (SkShader::kLinear_GradientType != shader->asAGradient(&grInfo)) {
        // TODO: non-linear gradient support
        return;
    }