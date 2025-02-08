sk_sp<SkFlattenable> SkLightingImageFilter::CreateProc(SkReadBuffer& buffer) {
    SK_IMAGEFILTER_UNFLATTEN_COMMON(common, 1);

    Light light;
    light.fType = buffer.read32LE(Light::Type::kLast);
    light.fLightColor = buffer.readColor();

    SkPoint3 lightPos, lightDir;
    buffer.readPoint3(&lightPos);
    light.fLocationXY = skif::ParameterSpace<SkPoint>({lightPos.fX, lightPos.fY});
    light.fLocationZ = skif::ParameterSpace<ZValue>(lightPos.fZ);

    buffer.readPoint3(&lightDir);
    light.fDirectionXY = skif::ParameterSpace<skif::Vector>({lightDir.fX, lightDir.fY});
    light.fDirectionZ = skif::ParameterSpace<ZValue>(lightDir.fZ);

    light.fFalloffExponent = buffer.readScalar();
    light.fCosCutoffAngle = buffer.readScalar();

    Material surfaceMaterial;
    surfaceMaterial.fType = buffer.read32LE(Material::Type::kLast);
    surfaceMaterial.fSurfaceDepth = skif::ParameterSpace<ZValue>(buffer.readScalar());
    surfaceMaterial.fK = buffer.readScalar();
    surfaceMaterial.fShininess = buffer.readScalar();

    if (!buffer.isValid()) {
        return nullptr;
    }

    return make_lighting(light, surfaceMaterial, common.getInput(0), common.cropRect());
}