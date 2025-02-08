switch (code)
  {
    case IMAGE_REL_ARM64_ABSOLUTE:
      return &arm64_reloc_howto_abs;
    case IMAGE_REL_ARM64_ADDR64:
      return &arm64_reloc_howto_64;
    case IMAGE_REL_ARM64_ADDR32:
      return &arm64_reloc_howto_32;
    case IMAGE_REL_ARM64_REL32:
      return &arm64_reloc_howto_32_pcrel;
    case IMAGE_REL_ARM64_BRANCH26:
      return &arm64_reloc_howto_branch26;
    case IMAGE_REL_ARM64_PAGEBASE_REL21:
      return &arm64_reloc_howto_page21;
    case IMAGE_REL_ARM64_REL21:
      return &arm64_reloc_howto_lo21;
    case IMAGE_REL_ARM64_PAGEOFFSET_12L:
      return &arm64_reloc_howto_pgoff12l;
    case IMAGE_REL_ARM64_BRANCH19:
      return &arm64_reloc_howto_branch19;
    case IMAGE_REL_ARM64_BRANCH14:
      return &arm64_reloc_howto_branch14;
    case IMAGE_REL_ARM64_PAGEOFFSET_12A:
      return &arm64_reloc_howto_pgoff12a;
    case IMAGE_REL_ARM64_ADDR32NB:
      return &arm64_reloc_howto_32nb;
    case IMAGE_REL_ARM64_SECREL:
      return &arm64_reloc_howto_secrel;
    default:
      BFD_FAIL ();
      return NULL;
  }

  return NULL;