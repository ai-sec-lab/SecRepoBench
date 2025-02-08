static byte *
pdfi_read_cff_real(byte *p, byte *e, float *val)
{
    char buf[65];
    char *txt = buf;

    /* b0 was 30 */

    while (txt < buf + (sizeof buf) - 5 && p < e) {
        int b, n;

        b = *p++;

        n = (b >> 4) &0xf;
        if (n < 0xA) {
            *txt++ = n + '0';
        }
        else if (n == 0xA) {
            *txt++ = '.';
        }
        else if (n == 0xB) {
            *txt++ = 'E';
        }
        else if (n == 0xC) {
            *txt++ = 'E';
            *txt++ = '-';
        }
        else if (n == 0xE) {
            *txt++ = '-';
        }
        else if (n == 0xF) {
            break;
        }

        n = b &0xf;
        if (n < 0xA) {
            *txt++ = n + '0';
        }
        else if (n == 0xA) {
            *txt++ = '.';
        }
        else if (n == 0xB) {
            *txt++ = 'E';
        }
        else if (n == 0xC) {
            *txt++ = 'E';
            *txt++ = '-';
        }
        else if (n == 0xE) {
            *txt++ = '-';
        }
        else if (n == 0xF) {
            break;
        }
    }

    *txt = 0;

    *val = atof(buf);

    return p;
}