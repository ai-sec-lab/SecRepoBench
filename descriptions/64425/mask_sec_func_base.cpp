void SipLayer::parseNextLayer()
{
	if (getLayerPayloadSize() == 0)
		return;

	size_t headerLen = getHeaderLen();
	// <MASK>
}