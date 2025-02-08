void Uart::ReceiveTask(const uint8_t *aBuf, uint16_t aBufLength)
{
    static const char sCommandPrompt[] = {'>', ' '};

#if OPENTHREAD_CONFIG_UART_CLI_RAW
    if (aBufLength > 0)
    {
        memcpy(mRxBuffer + mRxLength, aBuf, aBufLength);
        mRxLength += aBufLength;
    }

    if (aBuf[aBufLength - 1] == '\r' || aBuf[aBufLength - 1] == '\n')
    {
        mRxBuffer[mRxLength] = '\0';
        ProcessCommand();
        Output(sCommandPrompt, sizeof(sCommandPrompt));
    }
#else // OPENTHREAD_CONFIG_UART_CLI_RAW
    static const char sEraseString[] = {'\b', ' ', '\b'};
    static const char CRNL[]         = {'\r', '\n'};
    const uint8_t *   end;

    end = aBuf + aBufLength;

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
        // <MASK>
        }
    }
#endif // OPENTHREAD_CONFIG_UART_CLI_RAW
}