if (!matrix.decomposeScale(&scale, nullptr)) {
                scale.set(SK_ScalarNearlyZero, SK_ScalarNearlyZero);
            }
            fBaseFrequency.set(baseFrequencyX * SkScalarInvert(scale.width()),
                               baseFrequencyY * SkScalarInvert(scale.height()));
            fTileSize.set(SkScalarRoundToInt(tileVec.fX), SkScalarRoundToInt(tileVec.fY));
            this->init(seed);
            if (!fTileSize.isEmpty()) {
                this->stitch();
            }

    #if SK_SUPPORT_GPU
            SkImageInfo info = SkImageInfo::MakeA8(kBlockSize, 1);
            SkPixmap permutationsPixmap(info, fLatticeSelector, info.minRowBytes());
            fPermutationsImage = SkImage::MakeFromRaster(permutationsPixmap, nullptr, nullptr);

            info = SkImageInfo::MakeN32Premul(kBlockSize, 4);
            SkPixmap noisePixmap(info, fNoise[0][0], info.minRowBytes());
            fNoiseImage = SkImage::MakeFromRaster(noisePixmap, nullptr, nullptr);

            info = SkImageInfo::MakeA8(256, 1);
            SkPixmap impPermutationsPixmap(info, improved_noise_permutations, info.minRowBytes());
            fImprovedPermutationsImage = SkImage::MakeFromRaster(impPermutationsPixmap, nullptr,
                                                                 nullptr);

            static uint8_t gradients[] = { 2, 2, 1, 0,
                                           0, 2, 1, 0,
                                           2, 0, 1, 0,
                                           0, 0, 1, 0,
                                           2, 1, 2, 0,
                                           0, 1, 2, 0,
                                           2, 1, 0, 0,
                                           0, 1, 0, 0,
                                           1, 2, 2, 0,
                                           1, 0, 2, 0,
                                           1, 2, 0, 0,
                                           1, 0, 0, 0,
                                           2, 2, 1, 0,
                                           1, 0, 2, 0,
                                           0, 2, 1, 0,
                                           1, 0, 0, 0 };
            info = SkImageInfo::MakeN32Premul(16, 1);
            SkPixmap gradPixmap(info, gradients, info.minRowBytes());
            fGradientImage = SkImage::MakeFromRaster(gradPixmap, nullptr, nullptr);
    #endif