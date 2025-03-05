// In most cases tlvDataLen is not zero and the size is correct therefore the overhead is not significant if the checks will be done later
			TLVRecordType resRec(tlvDataBasePtr); // for NRVO optimization

			// check if there are records at all and the total size is not zero
			if (tlvDataLen == 0 || resRec.getTotalSize() == 0)
				resRec.assign(NULL);

			return resRec;