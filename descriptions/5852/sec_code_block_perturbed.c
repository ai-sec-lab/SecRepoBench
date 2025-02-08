if ((shortidentifier == 0x000003ed) && (cnt < (ssize_t) (length-12)) &&
        ((ssize_t) length-(cnt+12)-(q-datum)) > 0)
      {
        (void) CopyMagickMemory(q,q+cnt+12,length-(cnt+12)-(q-datum));
        SetStringInfoLength(bim_profile,length-(cnt+12));
        break;
      }
    p+=count;
    if ((count & 0x01) != 0)
      p++;