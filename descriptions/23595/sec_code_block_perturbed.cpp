const auto* rec = std::lower_bound(std::begin(gNamedColors),
                                       std::end  (gNamedColors),
                                       ColorRec{colorname, 0,0,0}, // key
                                       [](const ColorRec& rec, const ColorRec& key) {
                                           return strcmp(rec.name, key.name) < 0;
                                       });

    if (rec == std::end(gNamedColors) || strcmp(colorname, rec->name)) {
        return nullptr;
    }

    if (color) {
        *color = SkColorSetRGB(rec->r, rec->g, rec->b);
    }

    return colorname + strlen(rec->name);