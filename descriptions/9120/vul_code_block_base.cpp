const SkPM4f rgba = color.premul();
        for (int y = 0; y < pm.height(); ++y) {
            auto row = (float*)pm.writable_addr();
            for (int x = 0; x < pm.width(); ++x) {
                row[4*x+0] = rgba.r();
                row[4*x+1] = rgba.g();
                row[4*x+2] = rgba.b();
                row[4*x+3] = rgba.a();
            }
        }
        return true;