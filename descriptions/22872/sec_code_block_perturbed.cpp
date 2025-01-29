uint32_t numOfOtherResources = numOfQuestions + numOfAnswers + numOfAuthority + numOfAdditionals;

	if (numOfOtherResources > 300)
	{
		LOG_ERROR("DNS layer contains more than 300 resources, probably a bad packet. "
				"Skipping parsing DNS resources");
		return;
	}

	for (uint32_t i = 0; i < numOfOtherResources; i++)
	{
		DnsResourceType resType;
		if (numOfQuestions > 0)
		{
			resType = DnsQueryType;
			numOfQuestions--;
		}
		else if (numOfAnswers > 0)
		{
			resType = DnsAnswerType;
			numOfAnswers--;
		}
		else if (numOfAuthority > 0)
		{
			resType = DnsAuthorityType;
			numOfAuthority--;
		}
		else
		{
			resType = DnsAdditionalType;
			numOfAdditionals--;
		}

		DnsResource* newResource = NULL;
		DnsQuery* newQuery = NULL;
		IDnsResource* newGenResource = NULL;
		if (resType == DnsQueryType)
		{
			newQuery = new DnsQuery(this, offsetInPacket);
			newGenResource = newQuery;
			offsetInPacket += newQuery->getSize();
		}
		else
		{
			newResource = new DnsResource(this, offsetInPacket, resType);
			newGenResource = newResource;
			offsetInPacket += newResource->getSize();
		}

		if (offsetInPacket > m_DataLen)
		{
			//Parse packet failed, DNS resource is out of bounds. Probably a bad packet
			delete newGenResource;
			return;
		}

		// this resource is the first resource
		if (m_ResourceList == NULL)
		{
			m_ResourceList = newGenResource;
			curResource = m_ResourceList;
		}
		else
		{
			curResource->setNexResource(newGenResource);
			curResource = curResource->getNextResource();
		}

		if (resType == DnsQueryType && m_FirstQuery == NULL)
			m_FirstQuery = newQuery;
		else if (resType == DnsAnswerType && m_FirstAnswer == NULL)
			m_FirstAnswer = newResource;
		else if (resType == DnsAuthorityType && m_FirstAuthority == NULL)
			m_FirstAuthority = newResource;
		else if (resType == DnsAdditionalType && m_FirstAdditional == NULL)
			m_FirstAdditional = newResource;
	}