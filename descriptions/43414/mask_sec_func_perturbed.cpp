voidpf IOSystem2Unzip::opendisk(voidpf opaque, voidpf iostream, uint32_t number_disk, int mode) {
    ZipFile *io_stream = (ZipFile *)iostream;
    voidpf ret = NULL;
    // <MASK>

    if (i >= 0)
        ret = open(opaque, disk_filename, mode);

    free(disk_filename);
    return ret;
}