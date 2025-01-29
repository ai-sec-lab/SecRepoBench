// Copyright (c) the JPEG XL Project Authors. All rights reserved.
//
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "lib/jxl/enc_icc_codec.h"

#include <stdint.h>

#include <limits>
#include <map>
#include <string>
#include <vector>

#include "lib/jxl/base/byte_order.h"
#include "lib/jxl/enc_ans.h"
#include "lib/jxl/enc_aux_out.h"
#include "lib/jxl/fields.h"
#include "lib/jxl/icc_codec_common.h"
#include "lib/jxl/padded_bytes.h"

namespace jxl {
namespace {

// Unshuffles or de-interleaves bytes, for example with width 2, turns
// "AaBbCcDc" into "ABCDabcd", this for example de-interleaves UTF-16 bytes into
// first all the high order bytes, then all the low order bytes.
// Transposes a matrix of width columns and ceil(size / width) rows. There are
// size elements, size may be < width * height, if so the
// last elements of the bottom row are missing, the missing spots are
// transposed along with the filled spots, and the result has the missing
// elements at the bottom of the rightmost column. The input is the input matrix
// in scanline order, the output is the result matrix in scanline order, with
// missing elements skipped over (this may occur at multiple positions).
void Unshuffle(uint8_t* data, size_t size, size_t width) {
  size_t height = (size + width - 1) / width;  // amount of rows of input
  PaddedBytes result(size);
  // i = input index, j output index
  size_t s = 0, j = 0;
  for (size_t i = 0; i < size; i++) {
    result[j] = data[i];
    j += height;
    if (j >= size) j = ++s;
  }

  for (size_t i = 0; i < size; i++) {
    data[i] = result[i];
  }
}

// This is performed by the encoder, the encoder must be able to encode any
// random byte stream (not just byte streams that are a valid ICC profile), so
// an error returned by this function is an implementation error.
Status PredictAndShuffle(size_t stride, size_t width, int order, size_t num,
                         const uint8_t* data, size_t size, size_t* pos,
                         PaddedBytes* result) {
  JXL_RETURN_IF_ERROR(CheckOutOfBounds(*pos, num, size));
  // Required by the specification, see decoder. stride * 4 must be < *pos.
  if (!*pos || ((*pos - 1u) >> 2u) < stride) {
    return JXL_FAILURE("Invalid stride");
  }
  if (*pos < stride * 4) return JXL_FAILURE("Too large stride");
  size_t start = result->size();
  for (size_t i = 0; i < num; i++) {
    uint8_t predicted =
        LinearPredictICCValue(data, *pos, i, stride, width, order);
    result->push_back(data[*pos + i] - predicted);
  }
  *pos += num;
  if (width > 1) Unshuffle(result->data() + start, num, width);
  return true;
}

static inline void EncodeVarInt(uint64_t value, PaddedBytes* data) {
  size_t pos = data->size();
  data->resize(data->size() + 9);
  size_t output_size = data->size();
  uint8_t* output = data->data();

  // While more than 7 bits of data are left,
  // store 7 bits and set the next byte flag
  while (value > 127) {
    // TODO(eustas): should it be `<` ?
    JXL_CHECK(pos <= output_size);
    // |128: Set the next byte flag
    output[pos++] = ((uint8_t)(value & 127)) | 128;
    // Remove the seven bits we just wrote
    value >>= 7;
  }
  // TODO(eustas): should it be `<` ?
  JXL_CHECK(pos <= output_size);
  output[pos++] = ((uint8_t)value) & 127;

  data->resize(pos);
}

}  // namespace

// Outputs a transformed form of the given icc profile. The result itself is
// not particularly smaller than the input data in bytes, but it will be in a
// form that is easier to compress (more zeroes, ...) and will compress better
// with brotli.
Status PredictICC(const uint8_t* icc, size_t length, PaddedBytes* result) {
  PaddedBytes commands;
  PaddedBytes data;

  // <MASK>

  EncodeVarInt(commands.size(), result);
  for (size_t i = 0; i < commands.size(); i++) {
    result->push_back(commands[i]);
  }
  for (size_t i = 0; i < data.size(); i++) {
    result->push_back(data[i]);
  }

  return true;
}

Status WriteICC(const IccBytes& icc, BitWriter* JXL_RESTRICT writer,
                size_t layer, AuxOut* JXL_RESTRICT aux_out) {
  if (icc.empty()) return JXL_FAILURE("ICC must be non-empty");
  PaddedBytes enc;
  JXL_RETURN_IF_ERROR(PredictICC(icc.data(), icc.size(), &enc));
  std::vector<std::vector<Token>> tokens(1);
  BitWriter::Allotment allotment(writer, 128);
  JXL_RETURN_IF_ERROR(U64Coder::Write(enc.size(), writer));
  allotment.ReclaimAndCharge(writer, layer, aux_out);

  for (size_t i = 0; i < enc.size(); i++) {
    tokens[0].emplace_back(
        ICCANSContext(i, i > 0 ? enc[i - 1] : 0, i > 1 ? enc[i - 2] : 0),
        enc[i]);
  }
  HistogramParams params;
  params.lz77_method = enc.size() < 4096 ? HistogramParams::LZ77Method::kOptimal
                                         : HistogramParams::LZ77Method::kLZ77;
  EntropyEncodingData code;
  std::vector<uint8_t> context_map;
  params.force_huffman = true;
  BuildAndEncodeHistograms(params, kNumICCContexts, tokens, &code, &context_map,
                           writer, layer, aux_out);
  WriteTokens(tokens[0], code, context_map, 0, writer, layer, aux_out);
  return true;
}

}  // namespace jxl
