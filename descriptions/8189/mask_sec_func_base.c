static INT Dot11DecryptScanForKeys(
    PDOT11DECRYPT_CONTEXT ctx,
    const guint8 *data,
    const guint mac_header_len,
    const guint tot_len,
    DOT11DECRYPT_SEC_ASSOCIATION_ID id
)
{
    const UCHAR *addr;
    guint bodyLength;
    PDOT11DECRYPT_SEC_ASSOCIATION sta_sa;
    PDOT11DECRYPT_SEC_ASSOCIATION sa;
    guint offset = 0;
    const guint8 dot1x_header[] = {
        0xAA,             /* DSAP=SNAP */
        0xAA,             /* SSAP=SNAP */
        0x03,             /* Control field=Unnumbered frame */
        0x00, 0x00, 0x00, /* Org. code=encaps. Ethernet */
        0x88, 0x8E        /* Type: 802.1X authentication */
    };
    const guint8 bt_dot1x_header[] = {
        0xAA,             /* DSAP=SNAP */
        0xAA,             /* SSAP=SNAP */
        0x03,             /* Control field=Unnumbered frame */
        0x00, 0x19, 0x58, /* Org. code=Bluetooth SIG */
        0x00, 0x03        /* Type: Bluetooth Security */
    };
    const guint8 tdls_header[] = {
        0xAA,             /* DSAP=SNAP */
        0xAA,             /* SSAP=SNAP */
        0x03,             /* Control field=Unnumbered frame */
        0x00, 0x00, 0x00, /* Org. code=encaps. Ethernet */
        0x89, 0x0D,       /* Type: 802.11 - Fast Roaming Remote Request */
        0x02,             /* Payload Type: TDLS */
        0X0C              /* Action Category: TDLS */
    };

    const EAPOL_RSN_KEY *pEAPKey;
#ifdef DOT11DECRYPT_DEBUG
#define MSGBUF_LEN 255
    CHAR msgbuf[MSGBUF_LEN];
#endif
    DOT11DECRYPT_DEBUG_TRACE_START("Dot11DecryptScanForKeys");

    /* Callers provide these guarantees, so let's make them explicit. */
    // <MASK>

    DOT11DECRYPT_DEBUG_TRACE_END("Dot11DecryptScanForKeys");
    return DOT11DECRYPT_RET_NO_VALID_HANDSHAKE;
}