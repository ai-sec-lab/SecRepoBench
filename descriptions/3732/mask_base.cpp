/*
    RawSpeed - RAW file decoder.

    Copyright (C) 2009-2014 Klaus Post
    Copyright (C) 2017 Axel Waggershauser

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "decompressors/Cr2Decompressor.h"
#include "common/Common.h"                // for uint32, unroll_loop, ushort16
#include "common/Point.h"                 // for iPoint2D
#include "common/RawImage.h"              // for RawImage, RawImageData
#include "decoders/RawDecoderException.h" // for ThrowRDE
#include "io/BitPumpJPEG.h"               // for BitPumpJPEG
#include <algorithm>                      // for move, copy_n
#include <cassert>                        // for assert
#include <numeric>                        // for accumulate

using std::copy_n;

namespace rawspeed {

void Cr2Decompressor::decodeScan()
{
  if (predictorMode != 1)
    ThrowRDE("Unsupported predictor mode.");

  if (slicesWidths.empty()) {
    const int slicesWidth = frame.w * frame.cps;
    if (slicesWidth > mRaw->dim.x)
      ThrowRDE("Don't know slicing pattern, and failed to guess it.");

    slicesWidths.push_back(slicesWidth);
  }

  bool isSubSampled = false;
  for (uint32 i = 0; i < frame.cps;  i++)
    isSubSampled = isSubSampled || frame.compInfo[i].superH != 1 ||
                   frame.compInfo[i].superV != 1;

  if (isSubSampled) {
    if (mRaw->isCFA)
      ThrowRDE("Cannot decode subsampled image to CFA data");

    if (mRaw->getCpp() != frame.cps)
      ThrowRDE("Subsampled component count does not match image.");

    if (frame.cps != 3)
      ThrowRDE("Unsupported number of subsampled components: %u", frame.cps);

    // see http://lclevy.free.fr/cr2/#sraw for overview table
    bool isSupported = frame.compInfo[0].superH == 2;

    isSupported = isSupported && (frame.compInfo[0].superV == 1 ||
                                  frame.compInfo[0].superV == 2);

    for (uint32 i = 1; i < frame.cps; i++)
      isSupported = isSupported && frame.compInfo[i].superH == 1 &&
                    frame.compInfo[i].superV == 1;

    if (!isSupported) {
      ThrowRDE("Unsupported subsampling ([[%u, %u], [%u, %u], [%u, %u]])",
               frame.compInfo[0].superH, frame.compInfo[0].superV,
               frame.compInfo[1].superH, frame.compInfo[1].superV,
               frame.compInfo[2].superH, frame.compInfo[2].superV);
    }

    if (frame.compInfo[0].superV == 2)
      decodeN_X_Y<3, 2, 2>(); // Cr2 sRaw1/mRaw
    else {
      assert(frame.compInfo[0].superV == 1);
      decodeN_X_Y<3, 2, 1>(); // Cr2 sRaw2/sRaw
    }
  } else {
    switch (frame.cps) {
    case 2:
      decodeN_X_Y<2, 1, 1>();
      break;
    case 4:
      decodeN_X_Y<4, 1, 1>();
      break;
    default:
      ThrowRDE("Unsupported number of components: %u", frame.cps);
    }
  }
}

void Cr2Decompressor::decode(std::vector<int> slicesWidths_)
{
  slicesWidths = move(slicesWidths_);
  AbstractLJpegDecompressor::decode();
}

// N_COMP == number of components (2, 3 or 4)
// X_S_F  == x/horizontal sampling factor (1 or 2)
// Y_S_F  == y/vertical   sampling factor (1 or 2)

template <int N_COMP, int X_S_F, int Y_S_F>
void Cr2Decompressor::decodeN_X_Y()
{
  auto ht = getHuffmanTables<N_COMP>();
  auto pred = getInitialPredictors<N_COMP>();
  auto predNext = reinterpret_cast<ushort16*>(mRaw->getDataUncropped(0, 0));

  BitPumpJPEG bitStream(input);

  uint32 pixelPitch = mRaw->pitch / 2; // Pitch in pixel
  if (frame.cps != 3 && frame.w * frame.cps > 2 * frame.h) {
    // Fix Canon double height issue where Canon doubled the width and halfed
    // the height (e.g. with 5Ds), ask Canon. frame.w needs to stay as is here
    // because the number of pixels after which the predictor gets updated is
    // still the doubled width.
    // see: FIX_CANON_HALF_HEIGHT_DOUBLE_WIDTH
    frame.h *= 2;
  }

  if (X_S_F == 2 && Y_S_F == 1)
  {
    // fix the inconsistent slice width in sRaw mode, ask Canon.
    for (auto& sliceWidth : slicesWidths)
      sliceWidth = sliceWidth * 3 / 2;
  }

  for (const auto& slicesWidth : slicesWidths) {
    if (slicesWidth > mRaw->dim.x)
      ThrowRDE("Slice is longer than image's height, which is unsupported.");
  }

  // <MASK>
}

} // namespace rawspeed
