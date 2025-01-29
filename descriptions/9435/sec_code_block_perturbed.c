info.scale=(Quantum *) AcquireQuantumMemory(MagickMax(length,MaxMap)+1,
          sizeof(*info.scale));
        if (info.scale == (Quantum *) NULL)
          ThrowDCMException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(info.scale,0,(MagickMax(length,MaxMap)+1)*
          sizeof(*info.scale));