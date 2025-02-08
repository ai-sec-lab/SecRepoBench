static byte *
pdfi_count_cff_index(byte *p, byte *e, int *countp)
{
    int count, offsize, last;

    if (p + 3 > e) {
        gs_throw(-1, "not enough data for index header");
        return 0;
    }

    count = u16(p);
    p += 2;
    *countp = count;

    if (count == 0)
        return p;

    offsize = *p++;

    if (offsize < 1 || offsize > 4) {
        gs_throw(-1, "corrupt index header");
        return 0;
    }

    if (p + count * offsize > e) {
        gs_throw(-1, "not enough data for index offset table");
        return 0;
    }

    p += count * offsize;
    last = uofs(p, offsize);
    p += offsize;
    p--;                        /* stupid offsets */

    // <MASK>
}