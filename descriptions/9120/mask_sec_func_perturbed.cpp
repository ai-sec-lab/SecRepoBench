bool SkPixmap::erase(const SkColor4f& originalColor, const SkIRect* subset) const {
    SkPixmap pm;
    if (subset) {
        if (!this->extractSubset(&pm, *subset)) {
            return false;
        }
    } else {
        pm = *this;
    }

    const SkColor4f color = originalColor.pin();

    if (pm.colorType() == kRGBA_F16_SkColorType) {
        const uint64_t half4 = color.premul().toF16();
        for (int y = 0; y < pm.height(); ++y) {
            sk_memset64(pm.writable_addr64(0, y), half4, pm.width());
        }
        return true;
    }

    if (pm.colorType() == kRGBA_F32_SkColorType) {
        // <MASK>
    }

    return pm.erase(color.toSkColor());
}