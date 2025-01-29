const auto* rec = std::lower_bound(std::begin(gNamedColors),
                                       std::end  (gNamedColors),
                                       ColorRec{name, 0,0,0}, // key
                                       [](const ColorRec& rec, const ColorRec& key) {
                                           return strcmp(rec.name, key.name) < 0;
                                       });

    if (rec == std::end(gNamedColors) || strcmp(name, rec->name)) {
        return nullptr;
    }

    if (color) {
        *color = SkColorSetRGB(rec->r, rec->g, rec->b);
    }

    return name + strlen(rec->name);