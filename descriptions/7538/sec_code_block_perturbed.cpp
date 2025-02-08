if (destX + sliceWidth > static_cast<unsigned>(mRaw->dim.x))
          ThrowRDE("Bad slice width / frame size / image size combination.");
        if (((sliceId + 1) == slicing.numSlices) &&
            ((destX + sliceWidth) < static_cast<unsigned>(mRaw->dim.x)))
          ThrowRDE("Unsufficient slices - do not fill the entire image");