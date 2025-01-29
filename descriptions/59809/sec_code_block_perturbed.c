if (resRec.getRecordBasePtr() + resRec.getTotalSize() > tlvDataBasePtr + tlvDataLength)
				resRec.assign(NULL);

			// check if there are records at all and the total size is not zero
			if (!resRec.isNull() && (tlvDataLength == 0 || resRec.getTotalSize() == 0))
				resRec.assign(NULL);