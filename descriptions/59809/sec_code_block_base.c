if (resRec.getRecordBasePtr() + resRec.getTotalSize() > tlvDataBasePtr + tlvDataLen)
				resRec.assign(NULL);

			// check if there are records at all and the total size is not zero
			if (!resRec.isNull() && (tlvDataLen == 0 || resRec.getTotalSize() == 0))
				resRec.assign(NULL);