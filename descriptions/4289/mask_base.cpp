/*
    RawSpeed - RAW file decoder.

    Copyright (C) 2009-2010 Klaus Post
    Copyright (C) 2014-2015 Pedro Côrte-Real
    Copyright (C) 2017 Roman Lebedev

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

#include "decompressors/SamsungV0Decompressor.h"
#include "common/Common.h"                // for ushort16, uint32, int32
#include "common/Point.h"                 // for iPoint2D
#include "common/RawImage.h"              // for RawImage, RawImageData
#include "decoders/RawDecoderException.h" // for ThrowRDE
#include "io/BitPumpMSB32.h"              // for BitPumpMSB32
#include "io/Buffer.h"                    // for Buffer
#include "io/ByteStream.h"                // for ByteStream
#include "io/Endianness.h"                // for Endianness, Endianness::li...
#include "tiff/TiffEntry.h"               // for TiffEntry
#include "tiff/TiffIFD.h"                 // for TiffIFD
#include "tiff/TiffTag.h"                 // for TiffTag, TiffTag::IMAGELENGTH
#include <algorithm>                      // for max
#include <cassert>                        // for assert
#include <iterator>                       // for advance, begin, end, next
#include <vector>                         // for vector

namespace rawspeed {

SamsungV0Decompressor::SamsungV0Decompressor(const RawImage& image,
                                             const TiffIFD* raw,
                                             const Buffer* mFile)
    : AbstractSamsungDecompressor(image) {
  const uint32 width = mRaw->dim.x;
  const uint32 height = mRaw->dim.y;

  if (width == 0 || height == 0 || width < 16 || width > 5546 || height > 3714)
    ThrowRDE("Unexpected image dimensions found: (%u; %u)", width, height);

  const TiffEntry* sliceOffsets = raw->getEntry(static_cast<TiffTag>(40976));
  if (sliceOffsets->type != TIFF_LONG || sliceOffsets->count != 1)
    ThrowRDE("Entry 40976 is corrupt");

  ByteStream bso(mFile, sliceOffsets->getU32(), 4 * height, Endianness::little);

  const uint32 offset = raw->getEntry(STRIPOFFSETS)->getU32();
  const uint32 count = raw->getEntry(STRIPBYTECOUNTS)->getU32();
  Buffer rbuf(mFile->getSubView(offset, count));
  ByteStream bsr(DataBuffer(rbuf, Endianness::little));

  computeStripes(bso, bsr);
}

// FIXME: this is very close to IiqDecoder::computeSripes()
void SamsungV0Decompressor::computeStripes(ByteStream bso, ByteStream bsr) {
  const uint32 height = mRaw->dim.y;

  std::vector<uint32> offsets;
  offsets.reserve(1 + height);
  for (uint32 y = 0; y < height; y++)
    offsets.emplace_back(bso.getU32());
  offsets.emplace_back(bsr.getSize());

  stripes.reserve(height);

  auto offset_iterator = std::begin(offsets);
  bsr.skipBytes(*offset_iterator);

  auto next_offset_iterator = std::next(offset_iterator);
  while (next_offset_iterator < std::end(offsets)) {
    if (*offset_iterator >= *next_offset_iterator)
      ThrowRDE("Line offsets are out of sequence or slice is empty.");

    const auto size = *next_offset_iterator - *offset_iterator;
    assert(size > 0);

    stripes.emplace_back(bsr.getStream(size));

    std::advance(offset_iterator, 1);
    std::advance(next_offset_iterator, 1);
  }

  assert(stripes.size() == height);
}

void SamsungV0Decompressor::decompress() const {
  for (int y = 0; y < mRaw->dim.y; y++)
    decompressStrip(y, stripes[y]);

  // Swap red and blue pixels to get the final CFA pattern
  for (int y = 0; y < mRaw->dim.y - 1; y += 2) {
    auto* topline = reinterpret_cast<ushort16*>(mRaw->getData(0, y));
    auto* bottomline = reinterpret_cast<ushort16*>(mRaw->getData(0, y + 1));

    for (int x = 0; x < mRaw->dim.x - 1; x += 2) {
      ushort16 temp = topline[1];
      topline[1] = bottomline[0];
      bottomline[0] = temp;

      topline += 2;
      bottomline += 2;
    }
  }
}

int32 SamsungV0Decompressor::calcAdj(BitPumpMSB32* bits, int b) {
  int32 adj = 0;
  if (b)
    adj = (static_cast<int32>(bits->getBits(b)) << (32 - b) >> (32 - b));
  return adj;
}

void SamsungV0Decompressor::decompressStrip(uint32 y,
                                            const ByteStream& bs) const {
  const uint32 width = mRaw->dim.x;
  // <MASK>
}

} // namespace rawspeed
