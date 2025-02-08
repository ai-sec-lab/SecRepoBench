static byte *
pdfi_count_cff_index(byte *ptr, byte *e, int *countp)
{
    int count, offsize, last;

    if (ptr + 3 > e) {
        gs_throw(-1, "not enough data for index header");
        return 0;
    }

    count = u16(ptr);
    ptr += 2;
    *countp = count;

    if (count == 0)
        return ptr;

    offsize = *ptr++;

    if (offsize < 1 || offsize > 4) {
        gs_throw(-1, "corrupt index header");
        return 0;
    }

    if (ptr + count * offsize > e) {
        gs_throw(-1, "not enough data for index offset table");
        return 0;
    }

    ptr += count * offsize;
    last = uofs(ptr, offsize);
    ptr += offsize;
    ptr--;                        /* stupid offsets */

    if (last < 0) {
        gs_throw(-1, "corrupt index");
        return 0;
    }

    if (ptr + last > e) {
        gs_throw(-1, "not enough data for index data");
        return 0;
    }

    ptr += last;

    return ptr;
}