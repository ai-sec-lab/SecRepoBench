size_t IDnsResource::decodeName(const char* encodedName, char* result, int iteration)
{
	size_t encodedNameLength = 0;
	size_t decodedNameLength = 0;
	char* resultPtr = result;
	resultPtr[0] = 0;

	size_t curOffsetInLayer = (uint8_t*)encodedName - m_DnsLayer->m_Data;
	if (curOffsetInLayer + 1 > m_DnsLayer->m_DataLen)
		return encodedNameLength;

	// <MASK>

	// remove the last "."
	if (resultPtr > result)
	{
		result[resultPtr - result - 1] = 0;
	}

	// add the last '\0' to encodedNameLength
	resultPtr[0] = 0;
	encodedNameLength++;

	return encodedNameLength;
}