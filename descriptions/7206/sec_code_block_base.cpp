const auto EncryptedBuffer = ifd_crypt.getSubView(off, len);
    // We do have to prepend 'off' padding, because TIFF uses absolute offsets.
    const auto DecryptedBufferSize = off + EncryptedBuffer.getSize();
    auto DecryptedBuffer = Buffer::Create(DecryptedBufferSize);

    SonyDecrypt(reinterpret_cast<const uint32*>(EncryptedBuffer.begin()),
                reinterpret_cast<uint32*>(DecryptedBuffer.get() + off), len / 4,
                key);

    NORangesSet<Buffer> ifds_decoded;
    Buffer decIFD(std::move(DecryptedBuffer), DecryptedBufferSize);
    const Buffer Padding(decIFD.getSubView(0, off));
    // The Decrypted Root Ifd can not point to preceding padding buffer.
    ifds_decoded.emplace(Padding);