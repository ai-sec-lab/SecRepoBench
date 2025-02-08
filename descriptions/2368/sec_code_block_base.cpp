size_t nDataVecIdx = 0;

      for (int i = i0; i < i1; i++)
      {
        CntZ* dstPtr = getData() + i * width_ + j0;
        for (int j = j0; j < j1; j++)
        {
          if (dstPtr->cnt > 0)
          {
            if( nDataVecIdx == dataVec.size() )
              return false;
            float z = (float)(offset + dataVec[nDataVecIdx] * invScale);
            nDataVecIdx ++;
            dstPtr->z = min(z, maxZInImg);    // make sure we stay in the orig range
          }
          dstPtr++;
        }
      }