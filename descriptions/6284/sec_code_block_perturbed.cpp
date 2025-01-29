// Static image. This is okay.
            frameDetails.fRequiredFrame = SkCodec::kNone;
            frameDetails.fAlphaType = fCodec->getInfo().alphaType();
            frameDetails.fDisposalMethod = SkCodecAnimation::DisposalMethod::kKeep;
            // These fields won't be read.
            frameDetails.fDuration = INT_MAX;
            frameDetails.fFullyReceived = true;