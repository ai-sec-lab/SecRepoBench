void EnergyScanServer::HandleScanResult(Mac::EnergyScanResult *aResult)
{
    VerifyOrExit(mActive);

    if (aResult)
    {
        VerifyOrExit(mScanResultsLength < OPENTHREAD_CONFIG_TMF_ENERGY_SCAN_MAX_RESULTS);
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
}