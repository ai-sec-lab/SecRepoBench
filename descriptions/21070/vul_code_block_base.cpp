VerifyOrExit(mActive);

    if (aResult)
    {
        mScanResults[mScanResultsLength++] = aResult->mMaxRssi;
    }
    else
    {
        // clear the lowest channel to scan
        mChannelMaskCurrent &= mChannelMaskCurrent - 1;

        if (mChannelMaskCurrent == 0)
        {
            mChannelMaskCurrent = mChannelMask;
            mCount--;
        }

        if (mCount)
        {
            mTimer.Start(mPeriod);
        }
        else
        {
            mTimer.Start(kReportDelay);
        }
    }

exit:
    return;