void Uart::ReceiveTask(const uint8_t *aBuf, uint16_t bufLen)
{
    static const char sCommandPrompt[] = {'>', ' '};

#if OPENTHREAD_CONFIG_UART_CLI_RAW
    if (bufLen > 0)
    {
        memcpy(mRxBuffer + mRxLength, aBuf, bufLen);
        mRxLength += bufLen;
    }

    if (aBuf[bufLen - 1] == '\r' || aBuf[bufLen - 1] == '\n')
    {
        mRxBuffer[mRxLength] = '\0';
        ProcessCommand();
        Output(sCommandPrompt, sizeof(sCommandPrompt));
    }
#else // OPENTHREAD_CONFIG_UART_CLI_RAW
    static const char sEraseString[] = {'\b', ' ', '\b'};
    static const char CRNL[]         = {'\r', '\n'};
    const uint8_t *   end;

    end = aBuf + bufLen;

    for (; aBuf < end; aBuf++)
    {
        switch (*aBuf)
        {
        case '\r':
        case '\n':
            Output(CRNL, sizeof(CRNL));

            if (mRxLength > 0)
            {
                mRxBuffer[mRxLength] = '\0';
                ProcessCommand();
            }

            Output(sCommandPrompt, sizeof(sCommandPrompt));

            break;

#if OPENTHREAD_POSIX

        case 0x04: // ASCII for Ctrl-D
            exit(EXIT_SUCCESS);
            break;
#endif

        case '\b':
        case 127:
            if (mRxLength > 0)
            {
                Output(sEraseString, sizeof(sEraseString));
                mRxBuffer[--mRxLength] = '\0';
            }

            break;

        default:
            if (mRxLength < kRxBufferSize - 1)
            {
                Output(reinterpret_cast<const char *>(aBuf), 1);
                mRxBuffer[mRxLength++] = static_cast<char>(*aBuf);
            }

            break;
        }
    }
#endif // OPENTHREAD_CONFIG_UART_CLI_RAW
}