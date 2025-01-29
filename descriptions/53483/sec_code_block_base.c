if (dat->byte != 0x15)
    {
      LOG_ERROR ("Wrong HEADER Section Locator Records at %lu", dat->byte)
      return DWG_ERR_INVALIDDWG;
    }
  assert (dat->byte == 0x15);
  dwg->header.numsections = bit_read_RL (dat);
  LOG_TRACE ("\nnum_sections: " FORMAT_RL " [RL]\n", dwg->header.numsections)
  if (!dwg->header.numsections) // ODA writes zeros.
    dwg->header.numsections = 6;
  if (dwg->header.numsections < 3)
    {
      LOG_ERROR ("Not enough sections: " FORMAT_RL, dwg->header.numsections);
      return DWG_ERR_INVALIDDWG;
    }
  if (dwg->header.numsections > 10)
    {
      LOG_ERROR ("Too many sections: " FORMAT_RL, dwg->header.numsections);
      return DWG_ERR_INVALIDDWG;
    }

  // So far seen 3-6 sections. Most emit only 3-5 sections.
  dwg->header.section = (Dwg_Section *)calloc (
      1, sizeof (Dwg_Section) * dwg->header.numsections);
  if (!dwg->header.section)
    {
      LOG_ERROR ("Out of memory");
      return DWG_ERR_OUTOFMEM;
    }
  /* section 0: header vars
   *         1: class section
   *         2: object map
   *         3: (R13c3 and later): 2nd header (special table, no sentinels)
   *         4: optional: MEASUREMENT
   *         5: optional: AuxHeader (no sentinels, since r13c3
   */
  for (j = 0; j < dwg->header.numsections; j++)
    {
      dwg->header.section[j].number = bit_read_RC (dat);
      dwg->header.section[j].address = bit_read_RL (dat);
      dwg->header.section[j].size = bit_read_RL (dat);
      if (j < 6)
        strcpy (dwg->header.section[j].name, section_names[j]);
      LOG_TRACE ("section[%u].number:  %4d [RC] %s\n", j,
                 (int)dwg->header.section[j].number, dwg->header.section[j].name)
      LOG_TRACE ("section[%u].address: %4u [RL]\n", j,
                 (unsigned)dwg->header.section[j].address)
      LOG_TRACE ("section[%u].size:    %4u [RL]\n", j,
                 (unsigned)dwg->header.section[j].size);
      if (dwg->header.section[j].address + dwg->header.section[j].size > dat->size)
        {
          LOG_ERROR ("section[%u] address or size overflow: %lu + %u > %lu", j,
                     dwg->header.section[j].address, dwg->header.section[j].size,
                     dat->size);
          return DWG_ERR_INVALIDDWG;
        }
    }

  // Check CRC up to now (note: ODA has a bug here)
  crc2 = bit_calc_CRC (0xC0C1, &dat->chain[0], dat->byte); // from 0 to now
  crc = bit_read_RS (dat);
  LOG_TRACE ("crc: %04X [RSx] from 0-%lu\n", crc, dat->byte - 2);
  if (crc != crc2)
    {
      LOG_ERROR ("Header CRC mismatch %04X <=> %04X", crc, crc2);
      error |= DWG_ERR_WRONGCRC;
    }

  if (bit_search_sentinel (dat, dwg_sentinel (DWG_SENTINEL_HEADER_END)))
    LOG_TRACE ("         HEADER (end):    %4u\n", (unsigned)dat->byte)

  /*-------------------------------------------------------------------------
   * Section 5 AuxHeader
   * R2000+, mostly redundant file header information
   */
  if (dwg->header.numsections == 6 && dwg->header.version >= R_13c3)
    {
      int i;
      Dwg_AuxHeader *_obj = &dwg->auxheader;
      Bit_Chain *hdl_dat = dat;
      BITCODE_BL end_address
          = dwg->header.section[SECTION_AUXHEADER_R2000].address
            + dwg->header.section[SECTION_AUXHEADER_R2000].size;

      obj = NULL;
      dat->byte = dwg->header.section[SECTION_AUXHEADER_R2000].address;
      LOG_TRACE ("\n"
                 "=======> AuxHeader:       %4u\n", (unsigned)dat->byte)
      LOG_TRACE ("         AuxHeader (end): %4u\n", (unsigned)end_address)
      if (dat->size < end_address)
        {
          LOG_ERROR ("Invalid AuxHeader size: buffer overflow")
          error |= DWG_ERR_SECTIONNOTFOUND;
        }
      else
        {
          BITCODE_BL old_size = dat->size;
          BITCODE_BL vcount;
          dat->size = end_address;
          // clang-format off
          #include "auxheader.spec"
          // clang-format on
          dat->size = old_size;
        }
    }

  /*-------------------------------------------------------------------------
   * Picture (Pre-R13C3)
   */

  if (bit_search_sentinel (dat, dwg_sentinel (DWG_SENTINEL_THUMBNAIL_BEGIN)))
    {
      unsigned long int start_address;

      dat->bit = 0;
      start_address = dat->byte;
      LOG_TRACE ("\n=======> Thumbnail:       %4u\n", (unsigned int)start_address - 16);
      if (dwg->header.thumbnail_address
          && dwg->header.thumbnail_address != dat->byte - 16)
        LOG_WARN ("Illegal HEADER.thumbnail_address: %i != %lu",
                  dwg->header.thumbnail_address, dat->byte - 16)
      dwg->header.thumbnail_address = dat->byte - 16;
      if (bit_search_sentinel (dat, dwg_sentinel (DWG_SENTINEL_THUMBNAIL_END)))
        {
          BITCODE_RL bmpsize;
          LOG_TRACE ("         Thumbnail (end): %4u\n",
                     (unsigned int)dat->byte)
          if ((dat->byte - 16) < start_address)
            {
              LOG_ERROR ("Illegal HEADER.thumbnail_size: %lu < %lu",
                         dat->byte - 16, start_address);
            }
          else
            {
              assert ((dat->byte - 16) >= start_address);
              dwg->thumbnail.size = (dat->byte - 16) - start_address;
              dwg->thumbnail.chain
                = (unsigned char *)calloc (dwg->thumbnail.size, 1);
              dwg->thumbnail.byte = 0;
              if (!dwg->thumbnail.chain)
                {
                  LOG_ERROR ("Out of memory");
                  return DWG_ERR_OUTOFMEM;
                }
              memcpy (dwg->thumbnail.chain, &dat->chain[start_address],
                      dwg->thumbnail.size);
              dat->byte += dwg->thumbnail.size;
              dwg_bmp (dwg, &bmpsize);
              if (bmpsize > dwg->thumbnail.size)
                LOG_ERROR ("BMP size overflow: %i > %lu\n", bmpsize, dwg->thumbnail.size)
            }
        }
    }

  /*-------------------------------------------------------------------------
   * Header Variables, section 0
   */

  LOG_INFO ("\n"
            "=======> Header Variables:         %4u\n",
            (unsigned int)dwg->header.section[SECTION_HEADER_R13].address)
  LOG_INFO ("         Header Variables   (end): %4u\n",
            (unsigned int)(dwg->header.section[SECTION_HEADER_R13].address
                           + dwg->header.section[SECTION_HEADER_R13].size))
  if (dwg->header.section[SECTION_HEADER_R13].address < 58
      || dwg->header.section[SECTION_HEADER_R13].address
                 + dwg->header.section[SECTION_HEADER_R13].size
             > dat->size)
    {
      LOG_ERROR ("Invalid Header section, skipped")
      error |= DWG_ERR_SECTIONNOTFOUND;
      goto classes_section;
    }
  // after sentinel
  dat->byte = pvz = dwg->header.section[SECTION_HEADER_R13].address + 16;
  // LOG_HANDLE ("@ 0x%lx.%lu\n", bit_position (dat)/8, bit_position (dat)%8);
  dwg->header_vars.size = bit_read_RL (dat);
  LOG_TRACE ("         Length: " FORMAT_RL " [RL]\n", dwg->header_vars.size)
  dat->bit = 0;

  error |= dwg_decode_header_variables (dat, dat, dat, dwg);

  // LOG_HANDLE ("@ 0x%lx.%lu\n", bit_position (dat)/8, bit_position (dat)%8);
  // check slack
  if (dat->bit || dat->byte != pvz + dwg->header_vars.size + 4)
    {
      unsigned char r = 8 - dat->bit;
      LOG_HANDLE (" padding: %ld byte, %d bits\n",
                  pvz + dwg->header_vars.size + 4 - dat->byte, r);
    }
  // Check CRC, hardcoded to 2 before end sentinel
  LOG_HANDLE (" crc pos: %lu\n", pvz + dwg->header_vars.size + 4);
  bit_set_position (dat, (pvz + dwg->header_vars.size + 4) * 8);
  crc = bit_read_RS (dat);
  LOG_TRACE ("crc: %04X [RSx] from %lu-%lu=%ld\n", crc, pvz, dat->byte - 2,
             dat->byte - 2 - pvz);
  crc2 = 0;
  // LOG_HANDLE ("@ 0x%lx\n", bit_position (dat)/8);
  // LOG_HANDLE ("HEADER_R13.address of size 0x%lx\n", pvz);
  // LOG_HANDLE ("HEADER_R13.size %d\n",
  // dwg->header.section[SECTION_HEADER_R13].size);
  // typical sizes: 400-599
  if (dwg->header.section[SECTION_HEADER_R13].size > 34
      && dwg->header.section[SECTION_HEADER_R13].size < 0xfff
      && pvz < dat->byte
      && pvz + dwg->header.section[SECTION_HEADER_R13].size < dat->size)
    {
      // not dwg->header_vars.size, rather -4 (minus the section_size).
      // section_size + data, i.e. minus the 2x sentinel (32) + crc itself (2)
      // if we would include the crc we would always get 0000
      BITCODE_RL crc_size = dwg->header.section[SECTION_HEADER_R13].size - 34;
      LOG_HANDLE (" calc Header crc size: " FORMAT_RL "\n", crc_size);
      crc2 = bit_calc_CRC (0xC0C1, &dat->chain[pvz], crc_size);
    }
  if (crc != crc2)
    {
      LOG_WARN ("Header Section[%ld] CRC mismatch %04X <=> %04X",
                (long)dwg->header.section[SECTION_HEADER_R13].number, crc,
                crc2);
      error |= DWG_ERR_WRONGCRC;
    }

  /*-------------------------------------------------------------------------
   * Classes, section 1
   */
classes_section:
  LOG_INFO ("\n"
            "=======> Classes (start): %4lu\n",
            (long)dwg->header.section[SECTION_CLASSES_R13].address)
  LOG_INFO ("         Classes (end)  : %4lu\n",
            (long)(dwg->header.section[SECTION_CLASSES_R13].address
                   + dwg->header.section[SECTION_CLASSES_R13].size))
  LOG_INFO ("         Length         : %4lu\n",
            (long)dwg->header.section[SECTION_CLASSES_R13].size)
  // check sentinel
  dat->byte = dwg->header.section[SECTION_CLASSES_R13].address;
  if (dat->byte +16 > dat->size ||
      dwg->header.section[SECTION_CLASSES_R13].address
      + dwg->header.section[SECTION_CLASSES_R13].size
             > dat->size)
    {
      LOG_ERROR ("Invalid Classes section, skipped")
      error |= DWG_ERR_SECTIONNOTFOUND;
      goto handles_section;
    }
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
  LOG_INSANE ("endpos: %lu", endpos);
  LOG_POS_ (INSANE);

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
      LOG_HANDLE ("number: " FORMAT_BS " [BS] ", klass->number); LOG_POS_ (HANDLE);
      klass->proxyflag = bit_read_BS (dat);
      LOG_HANDLE ("proxyflag: " FORMAT_BS " [BS] ", klass->proxyflag); LOG_POS_ (HANDLE);
      if (dat->byte >= endpos)
        break;
      klass->appname = bit_read_TV (dat);
      LOG_HANDLE ("appname: %s [TV] ", klass->appname); LOG_POS_ (HANDLE);
      if (dat->byte >= endpos)
        {
          free (klass->appname);
          break;
        }
      LOG_HANDLE ("\n  ");
      klass->cppname = bit_read_TV (dat);
      LOG_HANDLE ("cppname: %s [TV] ", klass->cppname); LOG_POS_ (HANDLE);
      klass->dxfname = bit_read_TV (dat);
      LOG_HANDLE ("dxfname: %s [TV] ", klass->dxfname); LOG_POS_ (HANDLE);
      klass->is_zombie = bit_read_B (dat); // was_a_proxy
      LOG_HANDLE ("is_zombie: " FORMAT_B " [B] ", klass->is_zombie); LOG_POS_ (HANDLE);
      // 1f2 for entities, 1f3 for objects
      klass->item_class_id = bit_read_BS (dat);
      LOG_HANDLE ("item_class_id: " FORMAT_BS " [BS]", klass->item_class_id); LOG_POS_ (HANDLE);
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
        LOG_HANDLE ("num_instances: " FORMAT_BL " [BL]", klass->num_instances); LOG_POS_ (HANDLE);
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
handles_section:
  dat->byte = dwg->header.section[SECTION_HANDLES_R13].address;
  dat->bit = 0;

  lastmap = dat->byte + dwg->header.section[SECTION_HANDLES_R13].size; // 4
  dwg->num_objects = 0;
  object_begin = dat->size;
  object_end = 0;
  LOG_INFO ("\n"
            "=======> Handles (start) : %8u\n",
            (unsigned int)dwg->header.section[SECTION_HANDLES_R13].address)
  LOG_INFO ("         Handles (end)   : %8u\n",
            (unsigned int)(dwg->header.section[SECTION_HANDLES_R13].address
                           + dwg->header.section[SECTION_HANDLES_R13].size))
  LOG_INFO ("         Length: %u\n",
            (unsigned int)dwg->header.section[SECTION_HANDLES_R13].size)

  do
    {
      long unsigned int last_offset = 0;
      long unsigned int last_handle = 0;
      long unsigned int oldpos = 0;
      long unsigned int maxh = (unsigned long)dwg->header.section[SECTION_HANDLES_R13].size << 1;
      BITCODE_BL max_handles = maxh < INT32_MAX ? (BITCODE_BL)maxh
        : dwg->header.section[SECTION_HANDLES_R13].size;
      int added;

      startpos = dat->byte;
      section_size = bit_read_RS_LE (dat);
      LOG_TRACE ("Handles page size: %u [RS_LE] @%lu\n", section_size, startpos);
      if (section_size > 2040)
        {
          LOG_ERROR ("Object-map section size greater than 2040!")
          return DWG_ERR_VALUEOUTOFBOUNDS;
        }

      while (dat->byte - startpos < section_size)
        {
          BITCODE_UMC handleoff;
          BITCODE_MC offset;
          //BITCODE_BL last_handle = dwg->num_objects
          //  ? dwg->object[dwg->num_objects - 1].handle.value : 0;

          oldpos = dat->byte;
          // The offset from the previous handle. default: 1, unsigned.
          // Basically how many objects have been deleted here.
          handleoff = bit_read_UMC (dat);
          // The offset from the previous address. default: obj->size, signed.
          offset = bit_read_MC (dat);

          if ((handleoff == 0) || (handleoff > (max_handles - last_handle)))
            {
              BITCODE_MC prevsize = dwg->num_objects
                ? dwg->object[dwg->num_objects - 1].size : 0L;
              LOG_WARN ("handleoff %lu looks wrong, max_handles %u - last_handle %lu = %lu (@%lu)",
                        handleoff, (unsigned)max_handles, last_handle, max_handles - last_handle,
                        oldpos);
              if (offset == 1 ||
                  (offset > 0 && offset < prevsize && prevsize > 0) ||
                  (offset < 0 && labs((long)offset) < prevsize && prevsize > 0))
                {
                  if (offset != prevsize)
                    LOG_WARN ("offset %ld looks wrong, should be prevsize %ld", offset, prevsize);
                  // offset = prevsize;
                  // LOG_WARN ("Recover invalid offset to %ld", offset);
                }
            }
          last_offset += offset;
          LOG_TRACE ("\nNext object: %lu ", (unsigned long)dwg->num_objects)
          LOG_TRACE ("Handleoff: %lX [UMC] Offset: " FORMAT_MC " [MC]",
                     handleoff, offset)
          LOG_HANDLE (" @%lu", last_offset)
          LOG_TRACE ("\n")

          if (dat->byte == oldpos)
            break;

          if (object_end < last_offset)
            object_end = last_offset;
          if (object_begin > last_offset)
            object_begin = last_offset;

          added = dwg_decode_add_object (dwg, dat, dat, last_offset);
          if (added > 0)
            error |= added; // else not added (skipped) or -1 for re-allocated
          if (dwg->num_objects)
            last_handle = dwg->object[dwg->num_objects - 1].handle.value;
          // LOG_HANDLE ("dat: @%lu.%u\n", dat->byte, dat->bit);
        }
      if (dat->byte == oldpos)
        break;

      // CRC on
      if (dat->bit > 0)
        {
          dat->byte += 1;
          dat->bit = 0;
        }

      crc = bit_read_RS_LE (dat);
      LOG_TRACE ("\nHandles page crc: %04X [RS_LE] (%lu-%lu = %u)\n", crc,
                 startpos, startpos + section_size, section_size);
      crc2 = bit_calc_CRC (0xC0C1, dat->chain + startpos, section_size);
      if (crc != crc2)
        {
          LOG_ERROR ("Handles Section[%ld] page CRC mismatch %04X <=> %04X",
                     (long)dwg->header.section[SECTION_HANDLES_R13].number,
                     crc, crc2);
          // fails with r14
          // if (dwg->header.version == R_2000)
          //  return DWG_ERR_WRONGCRC;
          if (dat->from_version != R_14)
            error |= DWG_ERR_WRONGCRC;
        }
      if (dat->byte >= lastmap)
        break;
    }
  while (section_size > 2);

  LOG_INFO ("Num objects: %lu\n", (unsigned long)dwg->num_objects)
  LOG_INFO ("\n"
            "=======> Last Object      : %8lu\n",
            (unsigned long)object_begin)
  dat->byte = object_end;
  object_begin = bit_read_MS (dat);
  LOG_TRACE ("last object size: %lu [MS] (@%lu)\n", object_begin, object_end)
  LOG_INFO ("         Last Object (end): %8lu\n",
            (unsigned long)(object_end + object_begin + 2))

  /*
   // TODO: if the previous Handleoff got corrupted somehow, read this handle map 
   // and try again.

   dat->byte = dwg->header.section[SECTION_HANDLES_R13].address - 2;
   // Unknown bitdouble inter object data and object map
   antcrc = bit_read_CRC (dat);
   LOG_TRACE("Address: %08u / Content: 0x%04X", dat->byte - 2, antcrc)

   // check CRC-on
   antcrc = 0xC0C1;
   do
   {
     startpos = dat->byte;
     sgdc[0] = bit_read_RC (dat);
     sgdc[1] = bit_read_RC (dat);
     section_size = (sgdc[0] << 8) | sgdc[1];
     section_size -= 2;
     dat->byte += section_size;
     crc = bit_read_CRC (dat);
     dat->byte -= 2;
     bit_write_CRC (dat, startpos, antcrc);
     dat->byte -= 2;
     crc2 = bit_read_CRC (dat);
     if (loglevel) HANDLER (OUTPUT, "Read: %X\nCreated: %X\t SEMO: %X\n",
                            crc, crc2, antcrc);
     //antcrc = crc;
   } while (section_size > 0);
   */

  /*-------------------------------------------------------------------------
   * Second header, section 3. R13c3-R2000 only.
   * But partially also since r2004.
   */

  if (bit_search_sentinel (dat, dwg_sentinel (DWG_SENTINEL_SECOND_HEADER_BEGIN)))
    {
      BITCODE_RL i;
      BITCODE_RC sig, sig2;
      BITCODE_BL vcount;
      long unsigned int pvzadr;
      struct _dwg_second_header *_obj = &dwg->second_header;
      obj = NULL;

      LOG_INFO ("\n"
                "=======> Second Header 3 (start): %8u\n",
                (unsigned int)dat->byte - 16)
      pvzadr = dat->byte;
      LOG_TRACE ("pvzadr: %lx\n", pvzadr)

      FIELD_RL (size, 0);
      FIELD_BLx (address, 0);

      if (dwg->header.numsections <= SECTION_2NDHEADER_R13)
        {
          LOG_WARN ("Only %d num_sections, but 2ndheader found, extending to 4",
                    dwg->header.numsections);
          dwg->header.numsections = SECTION_2NDHEADER_R13 + 1; /* 4 */
          dwg->header.section = (Dwg_Section *)realloc (
              dwg->header.section,
              dwg->header.numsections * sizeof (Dwg_Section));
          memset (&dwg->header.section[SECTION_2NDHEADER_R13], 0,
                  sizeof (Dwg_Section));
        }
      if (!dwg->header.section[SECTION_2NDHEADER_R13].address)
        {
          dwg->header.section[SECTION_2NDHEADER_R13].address
              = dwg->second_header.address;
          dwg->header.section[SECTION_2NDHEADER_R13].size
              = dwg->second_header.size;
        }

      // AC1012, AC1013, AC1014 or AC1015. This is a char[11], zero padded.
      // with \n at 12.
      bit_read_fixed (dat, _obj->version, 12);
      LOG_TRACE ("version: %s [TFF 12]\n", _obj->version)
      for (i = 0; i < 4; i++)
        FIELD_B (null_b[i], 0);
      // DEBUG_HERE;
      // documented as 0x18,0x78,0x01,0x04 for R13, 0x18,0x78,0x01,0x05 for R14
      // r14:      7d f4 78 01
      // r2000:    14 64 78 01
      FIELD_RC (unknown_10, 0); // 0x10
      for (i = 0; i < 4; i++)
        FIELD_RC (unknown_rc4[i], 0);

      if (dat->from_version < R_2000 && FIELD_VALUE (unknown_10) == 0x18
          && FIELD_VALUE (unknown_rc4[0]) == 0x78)
        dat->byte -= 2;
      UNTIL (R_2000)
      {
        FIELD_RC (num_sections, 0); // r14: 5, r2000: 6
        for (i = 0; i < MIN (6, FIELD_VALUE (num_sections)); i++)
          {
            // address+sizes of sections 0-2 is correct, 3+4 is empty
            FIELD_RC (section[i].nr, 0);
            FIELD_BLx (section[i].address, 0);
            FIELD_BL (section[i].size, 0);
          }
        if (DWG_LOGLEVEL >= DWG_LOGLEVEL_HANDLE)
          {
            LOG_HANDLE ("1st header was:\n");
            for (i = 0; i < dwg->header.numsections; i++)
              {
                LOG_HANDLE ("section[" FORMAT_RL "] " FORMAT_RLd " " FORMAT_RLL
                            " " FORMAT_RL " \n",
                            i, dwg->header.section[i].number,
                            dwg->header.section[i].address,
                            dwg->header.section[i].size);
              }
            LOG_HANDLE ("start 3: %lu\n", pvzadr - 16);
          }

        FIELD_BS (num_handlers, 0); // 14, resp. 16 in r14
        if (FIELD_VALUE (num_handlers) > 16)
          {
            LOG_ERROR ("Second header num_handlers > 16: %d\n",
                       FIELD_VALUE (num_handlers));
            FIELD_VALUE (num_handlers) = 14;
          }
        for (i = 0; i < FIELD_VALUE (num_handlers); i++)
          {
            FIELD_RC (handlers[i].size, 0);
            FIELD_RC (handlers[i].nr, 0);
            FIELD_VECTOR (handlers[i].data, RC, handlers[i].size, 0);
          }

        // TODO: CRC check
        crc = bit_read_CRC (dat);
        LOG_TRACE ("crc: %04X\n", crc);

        VERSION (R_14)
        {
          FIELD_RL (junk_r14_1, 0);
          FIELD_RL (junk_r14_2, 0);
        }
      }

      if (bit_search_sentinel (dat, dwg_sentinel (DWG_SENTINEL_SECOND_HEADER_END)))
        LOG_INFO ("         Second Header 3 (end)  : %8u\n",
                  (unsigned int)dat->byte)
    }

  /*-------------------------------------------------------------------------
   * Section 4: MEASUREMENT (Called PADDING section in the ODA)
   */

  if (dwg->header.numsections > 4)
    {
      BITCODE_RL v;
      LOG_INFO ("\n"
                "=======> MEASUREMENT 4 (start)  : %8u\n",
                (unsigned int)dwg->header.section[4].address)
      LOG_INFO ("         MEASUREMENT 4 (end)    : %8u\n",
                (unsigned int)(dwg->header.section[4].address
                               + dwg->header.section[4].size))
      dat->byte = dwg->header.section[4].address;
      dat->bit = 0;
      v = bit_read_RL_LE (dat);
      LOG_TRACE ("MEASUREMENT: " FORMAT_RL " [RL_LE] (0 English/256 Metric)\n", v);
      dwg->header_vars.MEASUREMENT = v ? 1 : 0;
      //LOG_TRACE ("         Size bytes :\t%lu\n", dat->size)
    }

  // step II of handles parsing: resolve pointers from handle value
  // XXX: move this somewhere else
  LOG_INFO ("\nnum_objects: %lu\n", (unsigned long)dwg->num_objects)
  LOG_TRACE ("num_object_refs: %lu\n", (unsigned long)dwg->num_object_refs)
  LOG_TRACE ("Resolving pointers from ObjectRef vector:\n")
  error |= resolve_objectref_vector (dat, dwg);
  return error;