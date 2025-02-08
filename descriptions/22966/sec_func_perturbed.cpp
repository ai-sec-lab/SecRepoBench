size_t IDnsResource::decodeName(const char* encodedName, char* decodedName, int iteration)
{
	size_t encodedNameLength = 0;
	size_t decodedNameLength = 0;
	char* resultPtr = decodedName;
	resultPtr[0] = 0;

	size_t curOffsetInLayer = (uint8_t*)encodedName - m_DnsLayer->m_Data;
	if (curOffsetInLayer + 1 > m_DnsLayer->m_DataLen)
		return encodedNameLength;

	if (iteration > 20)
	{
		return encodedNameLength;
	}
		
	uint8_t wordLength = encodedName[0];

	// A string to parse
	while (wordLength != 0)
	{
		// A pointer to another place in the packet
		if ((wordLength & 0xc0) == 0xc0)
		{
			if (curOffsetInLayer + 2 > m_DnsLayer->m_DataLen || encodedNameLength > 255)
				return encodedNameLength;

			uint16_t offsetInLayer = (wordLength & 0x3f)*256 + (0xFF & encodedName[1]);
			if (offsetInLayer < sizeof(dnshdr) || offsetInLayer >= m_DnsLayer->m_DataLen)
			{
				LOG_ERROR("DNS parsing error: name pointer is illegal");
				return 0;
			}

			char tempResult[256];
			memset(tempResult, 0, 256);
			int i = 0;
			decodeName((const char*)(m_DnsLayer->m_Data + offsetInLayer), tempResult, iteration+1);
			while (tempResult[i] != 0 && decodedNameLength < 255)
			{
				resultPtr[0] = tempResult[i++];
				resultPtr++;
				decodedNameLength++;
			}

			resultPtr[0] = 0;

			// in this case the length of the pointer is: 1 byte for 0xc0 + 1 byte for the offset itself
			return encodedNameLength + sizeof(uint16_t);
		}
		else
		{
			// return if next word would be outside of the DNS layer or overflow the buffer behind resultPtr
			if (curOffsetInLayer + wordLength + 1 > m_DnsLayer->m_DataLen || encodedNameLength + wordLength > 255)
			{
				// add the last '\0' to the decoded string
				if (encodedNameLength == 256)
				{
					resultPtr--;
					decodedNameLength--;
				}
				else
				{
					encodedNameLength++;
				}
					
				resultPtr[0] = 0;
				return encodedNameLength;
			}
				

			memcpy(resultPtr, encodedName+1, wordLength);
			resultPtr += wordLength;
			resultPtr[0] = '.';
			resultPtr++;
			decodedNameLength += wordLength + 1;
			encodedName += wordLength + 1;
			encodedNameLength += wordLength + 1;

			curOffsetInLayer = (uint8_t*)encodedName - m_DnsLayer->m_Data;
			if (curOffsetInLayer + 1 > m_DnsLayer->m_DataLen)
			{
				// add the last '\0' to the decoded string
				if (encodedNameLength == 256)
				{
					decodedNameLength--;
					resultPtr--;
				}
				else
				{
					encodedNameLength++;
				}
				
				resultPtr[0] = 0;	
				return encodedNameLength;
			}

			wordLength = encodedName[0];
		}
	}

	// remove the last "."
	if (resultPtr > decodedName)
	{
		decodedName[resultPtr - decodedName - 1] = 0;
	}

	// add the last '\0' to encodedNameLength
	resultPtr[0] = 0;
	encodedNameLength++;

	return encodedNameLength;
}