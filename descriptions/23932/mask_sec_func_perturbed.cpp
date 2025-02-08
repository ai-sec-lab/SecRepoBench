void SkSVGDevice::AutoElement::addGradientShaderResources(const SkShader* shader,
                                                          const SkPaint& paint,
                                                          Resources* resources) {
    // <MASK>

    SkAutoSTArray<16, SkColor>  grColors(gradientInfo.fColorCount);
    SkAutoSTArray<16, SkScalar> grOffsets(gradientInfo.fColorCount);
    gradientInfo.fColors = grColors.get();
    gradientInfo.fColorOffsets = grOffsets.get();

    // One more call to get the actual colors/offsets.
    shader->asAGradient(&gradientInfo);
    SkASSERT(gradientInfo.fColorCount <= grColors.count());
    SkASSERT(gradientInfo.fColorCount <= grOffsets.count());

    resources->fPaintServer.printf("url(#%s)", addLinearGradientDef(gradientInfo, shader).c_str());
}