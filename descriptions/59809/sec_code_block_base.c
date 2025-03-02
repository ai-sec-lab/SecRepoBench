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
			TLVRecordType resRec(tlvDataBasePtr); // for NRVO optimization

			// resRec pointer is out-bounds of the TLV records memory
			if (resRec.getRecordBasePtr() + resRec.getTotalSize() > tlvDataBasePtr + tlvDataLen)
				resRec.assign(NULL);

			// check if there are records at all and the total size is not zero
			if (!resRec.isNull() && (tlvDataLen == 0 || resRec.getTotalSize() == 0))
				resRec.assign(NULL);

			return resRec;
		}