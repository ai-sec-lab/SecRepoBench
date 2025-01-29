if (memcmp (dwg_sentinel (DWG_SENTINEL_CLASS_BEGIN), &dat->chain[dat->byte], 16) == 0)
    dat->byte += 16;
  else
    LOG_TRACE ("no class sentinel\n");
  dat->bit = 0;
  size = bit_read_RL (dat);
  LOG_TRACE ("         Size : %lu [RL]\n", size)
  if (size != dwg->header.section[SECTION_CLASSES_R13].size - 38)
    {
      endpos = dwg->header.section[SECTION_CLASSES_R13].address
             + dwg->header.section[SECTION_CLASSES_R13].size - 16;
      LOG_WARN ("Invalid size %lu, endpos: %lu\n", size, endpos)
    }
  else
    endpos = dat->byte + size;
  LOG_INSANE ("endpos: %lu", endpos); LOG_POS;

  /* Read the classes
   */
  dwg->layout_type = 0;
  dwg->num_classes = 0;
#if 0
  SINCE (R_2004) { // dead code. looks better than the current.
    BITCODE_B btrue;
    BITCODE_BL max_num = bit_read_BL (dat);
    LOG_TRACE ("2004 max_num: " FORMAT_BL " [BL]\n", max_num);
    btrue = bit_read_B (dat); // always 1
    LOG_TRACE ("2004 btrue: " FORMAT_B " [B]\n", btrue);
  }
#endif
  while (dat->byte < endpos - 1)
    {
      BITCODE_BS i;
      Dwg_Class *klass;

      i = dwg->num_classes;
      if (i == 0)
        dwg->dwg_class = (Dwg_Class *)malloc (sizeof (Dwg_Class));
      else
        dwg->dwg_class = (Dwg_Class *)realloc (dwg->dwg_class,
                                               (i + 1) * sizeof (Dwg_Class));
      if (!dwg->dwg_class)
        {
          LOG_ERROR ("Out of memory");
          return DWG_ERR_OUTOFMEM;
        }
      klass = &dwg->dwg_class[i];
      memset (klass, 0, sizeof (Dwg_Class));
      klass->number = bit_read_BS (dat);
      LOG_HANDLE ("number: " FORMAT_BS " [BS] ", klass->number); LOG_POS;
      klass->proxyflag = bit_read_BS (dat);
      LOG_HANDLE ("proxyflag: " FORMAT_BS " [BS] ", klass->proxyflag); LOG_POS;
      if (dat->byte >= endpos)
        break;
      klass->appname = bit_read_TV (dat);
      LOG_HANDLE ("appname: %s [TV] ", klass->appname); LOG_POS;
      if (dat->byte >= endpos)
        break;
      LOG_HANDLE ("\n  ");
      klass->cppname = bit_read_TV (dat);
      LOG_HANDLE ("cppname: %s [TV] ", klass->cppname); LOG_POS;
      klass->dxfname = bit_read_TV (dat);
      LOG_HANDLE ("dxfname: %s [TV] ", klass->dxfname); LOG_POS;
      klass->is_zombie = bit_read_B (dat); // was_a_proxy
      LOG_HANDLE ("is_zombie: " FORMAT_B " [B] ", klass->is_zombie); LOG_POS;
      // 1f2 for entities, 1f3 for objects
      klass->item_class_id = bit_read_BS (dat);
      LOG_HANDLE ("item_class_id: " FORMAT_BS " [BS]", klass->item_class_id); LOG_POS;
      LOG_HANDLE ("\n");
      if (DWG_LOGLEVEL == DWG_LOGLEVEL_TRACE)
        {
          LOG (TRACE, "Class %d 0x%x %s\n"
                 " %s \"%s\" %d 0x%x\n",
                 klass->number, klass->proxyflag, klass->dxfname,
                 klass->cppname, klass->appname, klass->is_zombie,
                 klass->item_class_id)
       }

#if 0
      SINCE (R_2007) //? dead code it seems. see read_2004_section_classes()
      {
        klass->num_instances = bit_read_BL (dat);
        LOG_HANDLE ("num_instances: " FORMAT_BL " [BL]", klass->num_instances); LOG_POS;
        klass->dwg_version = bit_read_BL (dat); // nope: class_version
        klass->maint_version = bit_read_BL (dat);
        klass->unknown_1 = bit_read_BL (dat);
        klass->unknown_2 = bit_read_BL (dat);
        LOG_TRACE (
            " num_instances: %d, dwg/maint version: %d/%d, unk: %d/%d\n",
            klass->num_instances, klass->dwg_version, klass->maint_version,
            klass->unknown_1, klass->unknown_2);
      }
#endif

      if (klass->dxfname && strEQc ((const char *)klass->dxfname, "LAYOUT"))
        dwg->layout_type = klass->number;

      dwg->num_classes++;
      if (dwg->num_classes > 500)
        {
          LOG_ERROR ("number of classes is greater than 500");
          break;
        }
    }

  // Check Section CRC
  dat->byte = dwg->header.section[SECTION_CLASSES_R13].address
              + dwg->header.section[SECTION_CLASSES_R13].size - 18;
  dat->bit = 0;
  pvz = dwg->header.section[SECTION_CLASSES_R13].address + 16;
  if (!bit_check_CRC (dat, pvz, 0xC0C1))
    error |= DWG_ERR_WRONGCRC;

  dat->byte += 16; //sentinel
  pvz = bit_read_RL (dat); // Unknown bitlong inter class and object
  LOG_TRACE ("unknown: 0x%04lx [RL] @%lu\n", pvz, dat->byte - 4)
  LOG_INFO ("Number of classes read: %u\n", dwg->num_classes)

  /*-------------------------------------------------------------------------
   * Object-map, section 2
   */

  dat->byte = dwg->header.section[SECTION_HANDLES_R13].address;