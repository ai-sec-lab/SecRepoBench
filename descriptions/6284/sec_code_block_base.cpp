// Static image. This is okay.
            frameInfo.fRequiredFrame = SkCodec::kNone;
            frameInfo.fAlphaType = fCodec->getInfo().alphaType();
            frameInfo.fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
            // These fields won't be read.
            frameInfo.fDuration = INT_MAX;
            frameInfo.fFullyReceived = true;