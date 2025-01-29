if (destX + sliceWidth > static_cast<unsigned>(mRaw->dim.x))
          ThrowRDE("Bad slice width / frame size / image size combination.");