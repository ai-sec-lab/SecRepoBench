void SkBaseDevice::drawTextRSXform(const void* text, size_t len,
                                   const SkRSXform xform[], const SkPaint& paint) {
    CountTextProc proc = nullptr;
    switch (paint.getTextEncoding()) {
        case SkPaint::kUTF8_TextEncoding:
            proc = SkUTF8_CountUTF8Bytes;
            break;
        case SkPaint::kUTF16_TextEncoding:
            proc = count_utf16;
            break;
        case SkPaint::kUTF32_TextEncoding:
            proc = return_4;
            break;
        case SkPaint::kGlyphID_TextEncoding:
            proc = return_2;
            break;
    }

    SkPaint localPaint(paint);
    SkShader* shader = paint.getShader();

    SkMatrix localM, currM;
    const void* stopText = (const char*)text + len;
    while ((const char*)text < (const char*)stopText) {
        localM.setRSXform(*xform++);
        currM.setConcat(this->ctm(), localM);
        SkAutoDeviceCTMRestore adc(this, currM);

        // We want to rotate each glyph by the rsxform, but we don't want to rotate "space"
        // (i.e. the shader that cares about the ctm) so we have to undo our little ctm trick
        // with a localmatrixshader so that the shader draws as if there was no change to the ctm.
        if (shader) {
            SkMatrix inverse;
            if (localM.invert(&inverse)) {
                localPaint.setShader(shader->makeWithLocalMatrix(inverse));
            } else {
                localPaint.setShader(nullptr);  // can't handle this xform
            }
        }

        int subLen = proc((const char*)text);
        this->drawText(text, subLen, 0, 0, localPaint);
        text = (const char*)text + subLen;
    }
}