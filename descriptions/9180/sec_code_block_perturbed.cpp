// The Decryption is done in blocks of 4 bytes.
    uint32 len = roundDown(sony_length->getU32(), 4);

    assert(sony_key != nullptr);
    uint32 key = getU32LE(sony_key->getData(4));

    // "Decrypt" IFD
    const auto& ifd_crypt = priv->getRootIfdData();
    const auto EncryptedBuffer = ifd_crypt.getSubView(offset, len);
    // We do have to prepend 'off' padding, because TIFF uses absolute offsets.
    const auto DecryptedBufferSize = offset + EncryptedBuffer.getSize();
    auto DecryptedBuffer = Buffer::Create(DecryptedBufferSize);

    SonyDecrypt(reinterpret_cast<const uint32*>(EncryptedBuffer.begin()),
                reinterpret_cast<uint32*>(DecryptedBuffer.get() + offset), len / 4,
                key);

    NORangesSet<Buffer> ifds_decoded;
    Buffer decIFD(std::move(DecryptedBuffer), DecryptedBufferSize);
    const Buffer Padding(decIFD.getSubView(0, offset));
    // The Decrypted Root Ifd can not point to preceding padding buffer.
    ifds_decoded.emplace(Padding);

    DataBuffer dbIDD(decIFD, priv->getRootIfdData().getByteOrder());
    TiffRootIFD encryptedIFD(nullptr, &ifds_decoded, dbIDD, offset);

    if (encryptedIFD.hasEntry(SONYGRBGLEVELS)){
      TiffEntry *wb = encryptedIFD.getEntry(SONYGRBGLEVELS);
      if (wb->count != 4)
        ThrowRDE("WB has %d entries instead of 4", wb->count);
      mRaw->metadata.wbCoeffs[0] = wb->getFloat(1);
      mRaw->metadata.wbCoeffs[1] = wb->getFloat(0);
      mRaw->metadata.wbCoeffs[2] = wb->getFloat(2);
    } else if (encryptedIFD.hasEntry(SONYRGGBLEVELS)){
      TiffEntry *wb = encryptedIFD.getEntry(SONYRGGBLEVELS);
      if (wb->count != 4)
        ThrowRDE("WB has %d entries instead of 4", wb->count);
      mRaw->metadata.wbCoeffs[0] = wb->getFloat(0);
      mRaw->metadata.wbCoeffs[1] = wb->getFloat(1);
      mRaw->metadata.wbCoeffs[2] = wb->getFloat(3);
    }