bool SkPixmap::erase(const SkColor4f& origColor, const SkIRect* subset) const {
    SkPixmap pm;
    if (subset) {
        if (!this->extractSubset(&pm, *subset)) {
            return false;
        }
    } else {
        pm = *this;
    }

    const SkColor4f color = origColor.pin();

    if (pm.colorType() == kRGBA_F16_SkColorType) {
        const uint64_t half4 = color.premul().toF16();
        for (int y = 0; y < pm.height(); ++y) {
            sk_memset64(pm.writable_addr64(0, y), half4, pm.width());
        }
        return true;
    }

    if (pm.colorType() == kRGBA_F32_SkColorType) {
        const SkPM4f rgba = color.premul();
        for (int y = 0; y < pm.height(); ++y) {
            auto row = (float*)pm.writable_addr(0, y);
            for (int x = 0; x < pm.width(); ++x) {
                row[4*x+0] = rgba.r();
                row[4*x+1] = rgba.g();
                row[4*x+2] = rgba.b();
                row[4*x+3] = rgba.a();
            }
        }
        return true;
    }

    return pm.erase(color.toSkColor());
}