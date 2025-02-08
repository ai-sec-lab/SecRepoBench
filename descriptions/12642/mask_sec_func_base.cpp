void Interpreter::ProcessService(int argc, char *argv[])
{
    otError error = OT_ERROR_NONE;

    VerifyOrExit(argc > 0, error = OT_ERROR_INVALID_ARGS);

    if (strcmp(argv[0], "add") == 0)
    {
        otServiceConfig cfg;
        // <MASK>

        SuccessOrExit(error = otServerAddService(mInstance, &cfg));
    }
    else if (strcmp(argv[0], "remove") == 0)
    {
        long enterpriseNumber = 0;

        VerifyOrExit(argc > 2, error = OT_ERROR_INVALID_ARGS);

        SuccessOrExit(error = ParseLong(argv[1], enterpriseNumber));

        SuccessOrExit(error = otServerRemoveService(mInstance, static_cast<uint32_t>(enterpriseNumber),
                                                    reinterpret_cast<uint8_t *>(argv[2]),
                                                    static_cast<uint8_t>(strlen(argv[2]))));
    }
    else
    {
        ExitNow(error = OT_ERROR_INVALID_ARGS);
    }

exit:
    AppendResult(error);
}