info.scale=(Quantum *) AcquireQuantumMemory(MagickMax(length,256),
          sizeof(*info.scale));
        if (info.scale == (Quantum *) NULL)
          ThrowDCMException(ResourceLimitError,"MemoryAllocationFailed");
        (void) memset(info.scale,0,MagickMax(length,256)*
          sizeof(*info.scale));