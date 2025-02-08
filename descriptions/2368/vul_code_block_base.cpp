unsigned int* srcPtr = &dataVec[0];

      for (int i = i0; i < i1; i++)
      {
        CntZ* dstPtr = getData() + i * width_ + j0;
        for (int j = j0; j < j1; j++)
        {
          if (dstPtr->cnt > 0)
          {
            float z = (float)(offset + *srcPtr++ * invScale);
            dstPtr->z = min(z, maxZInImg);    // make sure we stay in the orig range
          }
          dstPtr++;
        }
      }