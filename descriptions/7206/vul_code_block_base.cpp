auto ifd_size = ifd_crypt.getSize();
    auto ifd_decoded = Buffer::Create(ifd_size);

    SonyDecrypt(reinterpret_cast<const uint32*>(ifd_crypt.getData(off, len)),
                reinterpret_cast<uint32*>(ifd_decoded.get() + off), len / 4,
                key);

    NORangesSet<Buffer> ifds_decoded;
    Buffer decIFD(move(ifd_decoded), ifd_size);