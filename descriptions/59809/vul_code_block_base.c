/**
		 * Get the first TLV record out of a byte stream
		 * @param[in] tlvDataBasePtr A pointer to the TLV data byte stream
		 * @param[in] tlvDataLen The TLV data byte stream length
		 * @return An instance of type TLVRecordType that contains the first TLV record. If tlvDataBasePtr is NULL or
		 * tlvDataLen is zero the returned TLVRecordType instance will be logically NULL, meaning TLVRecordType.isNull() will
		 * return true
		 */
		TLVRecordType getFirstTLVRecord(uint8_t* tlvDataBasePtr, size_t tlvDataLen) const
		{
			// In most cases tlvDataLen is not zero and the size is correct therefore the overhead is not significant if the checks will be done later
			TLVRecordType resRec(tlvDataBasePtr); // for NRVO optimization

			// check if there are records at all and the total size is not zero
			if (tlvDataLen == 0 || resRec.getTotalSize() == 0)
				resRec.assign(NULL);

			return resRec;
		}