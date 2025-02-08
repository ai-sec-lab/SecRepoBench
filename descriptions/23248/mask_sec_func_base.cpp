void NullLoopbackLayer::parseNextLayer()
{
	uint8_t* payload = m_Data + sizeof(uint32_t);
	size_t payloadLen = m_DataLen - sizeof(uint32_t);

	// <MASK>
}