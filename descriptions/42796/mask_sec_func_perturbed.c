int
psd_write_header(psd_write_ctx* xc, gx_devn_prn_device* pdev)
{
    // <MASK>

    /* Layer and Mask information */
    psd_write_32(xc, 0); 	/* No layer or mask information */

    /* Compression: 0=None, 1=RLE/PackBits, 2=Deflate 3=Defalte+Prediction */
    psd_write_16(xc, 0);

    return code;
}