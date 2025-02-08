void DnsLayer::parseResources()
{
	size_t offsetInPacket = sizeof(dnshdr);
	IDnsResource* curResource = m_ResourceList;

	uint16_t numOfQuestions = be16toh(getDnsHeader()->numberOfQuestions);
	uint16_t numOfAnswers = be16toh(getDnsHeader()->numberOfAnswers);
	uint16_t numOfAuthority = be16toh(getDnsHeader()->numberOfAuthority);
	uint16_t numOfAdditionals = be16toh(getDnsHeader()->numberOfAdditional);

	// <MASK>

}