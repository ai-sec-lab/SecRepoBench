int
decode_preR13_entities (BITCODE_RL start, BITCODE_RL end,
                        unsigned num_entities, BITCODE_RL size,
                        Bit_Chain *restrict bitchain, Dwg_Data *restrict dwg,
                        const EntitySectionIndexR11 entity_section)
{
  int error = 0;
  BITCODE_BL num = dwg->num_objects;
  BITCODE_RL real_start = start;
  size_t oldpos;
  BITCODE_RLL hdr_handle = 0;
  const char *entities_section[]
      = { "entities", "blocks entities", "extras entities" };
  Dwg_Object *hdr = NULL;
  Dwg_Object_BLOCK_HEADER *_hdr = NULL;
  BITCODE_BL block_idx = 0, hdr_index = 0;

  LOG_TRACE ("\n%s: (" FORMAT_RLx "-" FORMAT_RLx " (%u), size " FORMAT_RL
             ")\n",
             entities_section[entity_section], start, end, num_entities, size);
  LOG_INFO ("==========================================\n");
  if (entity_section != BLOCKS_SECTION_INDEX)
    {
      hdr = dwg_model_space_object (dwg);
      if (hdr && hdr->fixedtype == DWG_TYPE_BLOCK_HEADER)
        {
          hdr_index = hdr->index;
          _hdr = hdr->tio.object->tio.BLOCK_HEADER;
          _hdr->block_offset_r11 = (BITCODE_RL)-1;
          if (!hdr->handle.value)
            hdr->handle.value = dwg_next_handle (dwg);
          hdr_handle = hdr->handle.value;
          LOG_TRACE ("owned by BLOCK %s (" FORMAT_RLLx ")\n", _hdr->name,
                     hdr_handle);
        }
    }
  // TODO search current offset in block_offset_r11 in BLOCK_HEADER's

  // with sentinel in case of R11
  SINCE (R_11)
  {
    real_start -= 16;
  }

  // report unknown data before entites block
  if (start != end && real_start > 0 && (BITCODE_RL)bitchain->byte != real_start)
    {
      LOG_WARN ("\n@0x%zx => start 0x%x", bitchain->byte, real_start);
      if ((BITCODE_RL)bitchain->byte < real_start)
        {
          if (real_start > bitchain->size)
            {
              UNKNOWN_UNTIL (bitchain->size);
            }
          else
            {
              UNKNOWN_UNTIL (real_start);
            }
        }
    }

  SINCE (R_11)
  {
    switch (entity_section)
      {
      case ENTITIES_SECTION_INDEX:
        DECODE_PRER13_SENTINEL (DWG_SENTINEL_R11_ENTITIES_BEGIN);
        break;
      case BLOCKS_SECTION_INDEX:
        DECODE_PRER13_SENTINEL (DWG_SENTINEL_R11_BLOCK_ENTITIES_BEGIN);
        break;
      case EXTRAS_SECTION_INDEX:
        DECODE_PRER13_SENTINEL (DWG_SENTINEL_R11_EXTRA_ENTITIES_BEGIN);
        break;
      default:
        LOG_ERROR ("Internal error: Illegal entity_section %d 0-2\n",
                   (int)entity_section);
        return DWG_ERR_INTERNALERROR;
      }
  }

  if (end > start && start == bitchain->byte)
    {
      oldpos = bitchain->byte;
      bitchain->bit = 0;
      while (bitchain->byte < oldpos + size)
        {
          Dwg_Object *obj;
          Dwg_Object_Type_r11 abstype;
          BITCODE_RC pline_flag;

          if (!num)
            dwg->object
                = (Dwg_Object *)calloc (REFS_PER_REALLOC, sizeof (Dwg_Object));
          else if (num >= dwg->num_alloced_objects)
            {
              while (num >= dwg->num_alloced_objects)
                dwg->num_alloced_objects *= 2;
              dwg->object = (Dwg_Object *)realloc (
                  dwg->object, dwg->num_alloced_objects * sizeof (Dwg_Object));
              LOG_TRACE ("REALLOC dwg->object vector to %u\n",
                         dwg->num_alloced_objects)
              dwg->dirty_refs = 1;
            }
          if (!dwg->object)
            {
              LOG_ERROR ("Out of memory");
              return DWG_ERR_OUTOFMEM;
            }
          obj = &dwg->object[num];
          memset (obj, 0, sizeof (Dwg_Object));
          dwg->num_objects++;
          obj->index = num;
          obj->parent = dwg;
          obj->address = bitchain->byte;
          obj->supertype = DWG_SUPERTYPE_ENTITY;

          LOG_HANDLE ("@offset 0x%zx\n", bitchain->byte - start);
          PRE (R_2_0b)
          {
            obj->type = bit_read_RS (bitchain);
            LOG_TRACE ("type: " FORMAT_RS " [RS]\n", obj->type);
            if (obj->type > 127)
              { // deleted. moved into BLOCK
                abstype = (Dwg_Object_Type_r11)abs ((int8_t)obj->type);
                LOG_TRACE ("deleted\n");
              }
            else
              {
                abstype = (Dwg_Object_Type_r11)obj->type;
              }
          }
          else
          {
            obj->type = bit_read_RC (bitchain);
            LOG_TRACE ("type: " FORMAT_RCd " [RCd]\n", obj->type);
            if (obj->type > 127)
              { // deleted. moved into BLOCK
                abstype = (Dwg_Object_Type_r11)((unsigned)obj->type & 0x7F);
                LOG_TRACE ("deleted\n");
              }
            else
              {
                abstype = (Dwg_Object_Type_r11)obj->type;
              }
          }

          switch (abstype)
            {
            case DWG_TYPE_LINE_r11:
              error |= dwg_decode_LINE (bitchain, obj);
              break;
            case DWG_TYPE_POINT_r11:
              error |= dwg_decode_POINT (bitchain, obj);
              break;
            case DWG_TYPE_CIRCLE_r11:
              error |= dwg_decode_CIRCLE (bitchain, obj);
              break;
            case DWG_TYPE_SHAPE_r11:
              error |= dwg_decode_SHAPE (bitchain, obj);
              break;
            case DWG_TYPE_REPEAT_r11:
              error |= dwg_decode_REPEAT (bitchain, obj);
              break;
            case DWG_TYPE_ENDREP_r11:
              error |= dwg_decode_ENDREP (bitchain, obj);
              break;
            case DWG_TYPE_TEXT_r11:
              error |= dwg_decode_TEXT (bitchain, obj);
              break;
            case DWG_TYPE_ARC_r11:
              error |= dwg_decode_ARC (bitchain, obj);
              break;
            case DWG_TYPE_TRACE_r11:
              error |= dwg_decode_TRACE (bitchain, obj);
              break;
            case DWG_TYPE_LOAD_r11:
              error |= dwg_decode_LOAD (bitchain, obj);
              break;
            case DWG_TYPE_SOLID_r11:
              error |= dwg_decode_SOLID (bitchain, obj);
              break;
            case DWG_TYPE_BLOCK_r11:
              {
                BITCODE_RL cur_offset;
                BITCODE_RL cur_offset_prefix = 0;
                if (bitchain->version > R_2_22)
                  cur_offset_prefix += 0x40000000;
                // search current offset in block_offset_r11 in BLOCK_HEADER's
                // and set new _hdr
                cur_offset = (((bitchain->byte - 1) - start) | cur_offset_prefix)
                             & 0xFFFFFFFF;
                error |= dwg_decode_BLOCK (bitchain, obj);
                if (!_hdr && entity_section == BLOCKS_SECTION_INDEX)
                  {
                    for (BITCODE_BL i = 0; i < dwg->num_objects; i++)
                      {
                        Dwg_Object *o = &dwg->object[i];
                        if (o->fixedtype == DWG_TYPE_BLOCK_HEADER
                            && o->tio.object && o->tio.object->tio.BLOCK_HEADER
                            && cur_offset
                                   == o->tio.object->tio.BLOCK_HEADER
                                          ->block_offset_r11)
                          {
                            LOG_TRACE ("found BLOCK_HEADER \"%s\" at "
                                       "block_offset_r11 "
                                       "0x%x\n",
                                       o->tio.object->tio.BLOCK_HEADER->name,
                                       cur_offset);
                            hdr = o;
                            hdr_handle = hdr->handle.value;
                            hdr_index = o->index;
                            _hdr = o->tio.object->tio.BLOCK_HEADER;
                            if (!obj->handle.value)
                              obj->handle.value = dwg_next_handle (dwg);
                            if (!_hdr->block_entity)
                              _hdr->block_entity = dwg_add_handleref (
                                  dwg, 3, obj->handle.value, obj);
                            else
                              {
                                _hdr->block_entity->handleref.code = 3;
                                _hdr->block_entity->absolute_ref
                                    = _hdr->block_entity->handleref.value
                                    = obj->handle.value;
                                _hdr->block_entity->obj = obj;
                              }
                            LOG_TRACE (
                                "BLOCK_HEADER.block_entity: " FORMAT_HREF11
                                "\n",
                                ARGS_HREF11 (_hdr->block_entity));
                            if (!obj->tio.entity->tio.BLOCK->name)
                              obj->tio.entity->tio.BLOCK->name
                                  = strdup (_hdr->name);
                            // LOG_TRACE ("next entities owned by BLOCK \"%s\"
                            // (%lx)\n",
                            //            _hdr->name, hdr_handle);
                            block_idx++;
                            break;
                          }
                      }
                    if (!_hdr)
                      {
                        const char *name
                            = obj->fixedtype == DWG_TYPE_BLOCK
                                  ? obj->tio.entity->tio.BLOCK->name
                                  : "(null)";
                        LOG_WARN (
                            "found no BLOCK_HEADER %s block_offset_r11 0x%x\n",
                            name, cur_offset);
                        hdr = NULL;
                      }
                  }
              }
              break;
            case DWG_TYPE_ENDBLK_r11:
              error |= dwg_decode_ENDBLK (bitchain, obj);
              if (_hdr)
                {
                  hdr = &dwg->object[hdr_index];
                  _hdr->endblk_entity
                      = dwg_add_handleref (dwg, 3, obj->handle.value, hdr);
                  LOG_TRACE ("BLOCK_HEADER.endblk_entity: " FORMAT_HREF11 "\n",
                             ARGS_HREF11 (_hdr->endblk_entity));
                }
              hdr = NULL;
              _hdr = NULL;
              break;
            case DWG_TYPE_INSERT_r11:
              error |= dwg_decode_INSERT (bitchain, obj);
              break;
            case DWG_TYPE_ATTDEF_r11:
              error |= dwg_decode_ATTDEF (bitchain, obj);
              break;
            case DWG_TYPE_ATTRIB_r11:
              error |= dwg_decode_ATTRIB (bitchain, obj);
              break;
            case DWG_TYPE_SEQEND_r11:
              error |= dwg_decode_SEQEND (bitchain, obj);
              break;
            case DWG_TYPE_JUMP_r11:
              error |= dwg_decode_JUMP (bitchain, obj);
              break;
            case DWG_TYPE_POLYLINE_r11:
              { // which polyline
                BITCODE_RC flag_r11;
                BITCODE_RS opts_r11;
                BITCODE_RC extra_r11 = 0;
                BITCODE_RS eed_size;
                BITCODE_RC handling_len;
                size_t start_byte;
                LOG_TRACE ("Detect polyline:");
                start_byte = bitchain->byte;
                LOG_TRACE (" start_byte: %" PRIuSIZE ",", start_byte);
                flag_r11 = bit_read_RC (bitchain);
                LOG_TRACE (" flag_r11: 0x%x,", flag_r11);
                bitchain->byte += 4;
                opts_r11 = bit_read_RS (bitchain);
                LOG_TRACE (" opts_r11: 0x%x", opts_r11);
                if (opts_r11 & OPTS_R11_POLYLINE_HAS_FLAG)
                  {
                    if (flag_r11 & FLAG_R11_HAS_PSPACE)
                      {
                        extra_r11 = bit_read_RC (bitchain);
                        LOG_TRACE (", extra_r11: 0x%x", extra_r11);
                      }
                    if (flag_r11 & FLAG_R11_HAS_COLOR)
                      bitchain->byte += 1;
                    if (flag_r11 & FLAG_R11_HAS_LTYPE)
                      {
                        PRE (R_11)
                        {
                          bitchain->byte += 1;
                        }
                        else bitchain->byte += 2;
                      }
                    if (flag_r11 & FLAG_R11_HAS_THICKNESS)
                      bitchain->byte += 8;
                    if (flag_r11 & FLAG_R11_HAS_ELEVATION)
                      bitchain->byte += 8;
                    if (extra_r11 & EXTRA_R11_HAS_EED)
                      {
                        eed_size = bit_read_RS (bitchain);
                        LOG_TRACE (", eed_size: %d", eed_size);
                        bitchain->byte += eed_size;
                      }
                    if (flag_r11 & FLAG_R11_HAS_HANDLING)
                      {
                        handling_len = bit_read_RC (bitchain);
                        LOG_TRACE (", handling_len: %d", handling_len);
                        bitchain->byte += handling_len;
                      }
                    if (extra_r11 & EXTRA_R11_HAS_VIEWPORT)
                      bitchain->byte += 2;
                    pline_flag = bit_read_RC (bitchain);
                    LOG_TRACE (", pline_flag: 0x%x", pline_flag);
                    LOG_POS;
                    bitchain->byte = start_byte;
                    if (pline_flag & FLAG_POLYLINE_3D)
                      error |= dwg_decode_POLYLINE_3D (bitchain, obj);
                    else if (pline_flag & FLAG_POLYLINE_MESH)
                      error |= dwg_decode_POLYLINE_MESH (bitchain, obj);
                    else if (pline_flag & FLAG_POLYLINE_PFACE_MESH)
                      error |= dwg_decode_POLYLINE_PFACE (bitchain, obj);
                    else
                      error |= dwg_decode_POLYLINE_2D (bitchain, obj);
                  }
                else
                  {
                    bitchain->byte = start_byte;
                    LOG_TRACE ("\n");
                    error |= dwg_decode_POLYLINE_2D (bitchain, obj);
                  }
              }
              break;
            case DWG_TYPE_VERTEX_r11:
              { // which vertex?
                BITCODE_RC flag_r11;
                BITCODE_RS opts_r11;
                BITCODE_RC extra_r11 = 0;
                BITCODE_RS eed_size;
                BITCODE_RC handling_len;
                BITCODE_RC vertex_flag;
                size_t start_byte;
                LOG_TRACE ("Detect vertex:");
                start_byte = bitchain->byte;
                LOG_TRACE (" start_byte: %" PRIuSIZE ",", start_byte);
                flag_r11 = bit_read_RC (bitchain);
                LOG_TRACE (" flag_r11: 0x%x,", flag_r11);
                bitchain->byte += 4;
                opts_r11 = bit_read_RS (bitchain);
                LOG_TRACE (" opts_r11: 0x%x", opts_r11);
                if (flag_r11 & FLAG_R11_HAS_COLOR)
                  bitchain->byte += 1;
                if (flag_r11 & FLAG_R11_HAS_LTYPE)
                  {
                    PRE (R_11)
                    {
                      bitchain->byte += 1;
                    }
                    else bitchain->byte += 2;
                  }
                if (flag_r11 & FLAG_R11_HAS_THICKNESS)
                  bitchain->byte += 8;
                if (flag_r11 & FLAG_R11_HAS_ELEVATION)
                  bitchain->byte += 8;
                if (flag_r11 & FLAG_R11_HAS_PSPACE)
                  {
                    extra_r11 = bit_read_RC (bitchain);
                    LOG_TRACE (", extra_r11: 0x%x", extra_r11);
                  }
                if (extra_r11 && extra_r11 & EXTRA_R11_HAS_EED)
                  {
                    eed_size = bit_read_RS (bitchain);
                    LOG_TRACE (", eed_size: %d", eed_size);
                    bitchain->byte += eed_size;
                  }
                if (flag_r11 & FLAG_R11_HAS_HANDLING)
                  {
                    handling_len = bit_read_RC (bitchain);
                    LOG_TRACE (", handling_len: %d", handling_len);
                    bitchain->byte += handling_len;
                  }
                if (extra_r11 && extra_r11 & EXTRA_R11_HAS_VIEWPORT)
                  bitchain->byte += 2;
                if (!(opts_r11 & OPTS_R11_VERTEX_HAS_NOT_X_Y))
                  bitchain->byte += 16;
                if (opts_r11 & OPTS_R11_VERTEX_HAS_START_WIDTH)
                  bitchain->byte += 8;
                if (opts_r11 & OPTS_R11_VERTEX_HAS_END_WIDTH)
                  bitchain->byte += 8;
                if (opts_r11 & OPTS_R11_VERTEX_HAS_BULGE)
                  bitchain->byte += 8;
                if (opts_r11 & OPTS_R11_VERTEX_HAS_FLAG)
                  {
                    vertex_flag = bit_read_RC (bitchain);
                    LOG_TRACE (", vertex_flag: 0x%x", vertex_flag);
                    LOG_POS;
                    bitchain->byte = start_byte;
                    if (vertex_flag & FLAG_VERTEX_MESH
                        && vertex_flag & FLAG_VERTEX_PFACE_MESH)
                      error |= dwg_decode_VERTEX_PFACE (bitchain, obj);
                    else if (vertex_flag & FLAG_VERTEX_MESH)
                      error |= dwg_decode_VERTEX_MESH (bitchain, obj);
                    else if (vertex_flag & FLAG_VERTEX_PFACE_MESH)
                      error |= dwg_decode_VERTEX_PFACE_FACE (bitchain, obj);
                    else if (vertex_flag & FLAG_VERTEX_3D)
                      error |= dwg_decode_VERTEX_3D (bitchain, obj);
                    else
                      error |= dwg_decode_VERTEX_2D (bitchain, obj);
                  }
                else
                  {
                    bitchain->byte = start_byte;
                    LOG_TRACE ("\n");
                    error |= dwg_decode_VERTEX_2D (bitchain, obj);
                  }
              }
              break;
            case DWG_TYPE_3DLINE_r11:
              error |= dwg_decode__3DLINE (bitchain, obj);
              break;
            case DWG_TYPE_3DFACE_r11:
              error |= dwg_decode__3DFACE (bitchain, obj);
              break;
            case DWG_TYPE_DIMENSION_r11:
              error |= decode_preR13_DIMENSION (bitchain, obj);
              break;
            case DWG_TYPE_VIEWPORT_r11:
              error |= dwg_decode_VIEWPORT (bitchain, obj);
              break;
            default:
              bitchain->byte--;
              DEBUG_HERE;
              LOG_ERROR ("Unknown object type %d", obj->type);
              error |= DWG_ERR_SECTIONNOTFOUND;
              bitchain->byte++;
              break;
            }

          assert (!bitchain->bit);
          PRE (R_2_0b)
          {
            obj->size = (bitchain->byte - oldpos) & 0xFFFFFFFF;
            if (num + 1 > dwg->num_objects)
              break;
          }
          if (obj->type != DWG_TYPE_JUMP_r11)
            {
              SINCE (R_2_0b) // Pre R_2_0 doesn't contain size of entity
              {
                PRE (R_11) // no crc16
                {
                  if (obj->size > bitchain->size - oldpos ||
                      obj->size + obj->address > bitchain->byte)
                    {
                      LOG_ERROR ("Invalid obj->size " FORMAT_RL " changed to %" PRIuSIZE,
                                 obj->size, bitchain->byte - oldpos);
                      error |= DWG_ERR_VALUEOUTOFBOUNDS;
                      obj->size = (bitchain->byte - oldpos) & 0xFFFFFFFF;
                    }
                  else if (obj->address + obj->size != bitchain->byte)
                    {
                      LOG_ERROR ("offset %ld",
                                 (long)(obj->address + obj->size - bitchain->byte));
                      if (obj->address + obj->size > bitchain->byte)
                        {
                          BITCODE_RL offset
                              = (BITCODE_RL)(obj->address + obj->size
                                             - bitchain->byte);
                          obj->num_unknown_rest = 8 * offset;
                          obj->unknown_rest = (BITCODE_TF)calloc (offset, 1);
                          if (obj->unknown_rest)
                            {
                              memcpy (obj->unknown_rest,
                                      &bitchain->chain[bitchain->byte], offset);
                              LOG_TRACE_TF (obj->unknown_rest, offset);
                            }
                          else
                            {
                              LOG_ERROR ("Out of memory");
                              obj->num_unknown_rest = 0;
                            }
                        }
                      if (obj->size > 2)
                        bitchain->byte = obj->address + obj->size;
                    }
                }
                LATER_VERSIONS
                {
                  if (obj->size > bitchain->size - oldpos ||
                      obj->size + obj->address > bitchain->byte)
                    {
                      LOG_ERROR ("Invalid obj->size " FORMAT_RL " changed to %" PRIuSIZE,
                                 obj->size, bitchain->byte + 2 - oldpos);
                      error |= DWG_ERR_VALUEOUTOFBOUNDS;
                      obj->size = ((bitchain->byte + 2) - oldpos) & 0xFFFFFFFF;
                    }
                  else if (obj->address + obj->size != bitchain->byte + 2)
                    {
                      LOG_ERROR ("offset %ld", (long)(obj->address + obj->size
                                                      - (bitchain->byte + 2)));
                      if (obj->address + obj->size > bitchain->byte + 2)
                        {
                          BITCODE_RL offset
                              = (BITCODE_RL)(obj->address + obj->size
                                             - (bitchain->byte + 2));
                          obj->num_unknown_rest = 8 * offset;
                          obj->unknown_rest = bit_read_TF (bitchain, offset);
                          if (obj->unknown_rest)
                            {
                              LOG_TRACE_TF (obj->unknown_rest, offset);
                            }
                          else
                            {
                              LOG_ERROR ("Out of memory");
                              obj->num_unknown_rest = 0;
                            }
                        }
                      if (obj->address + obj->size >= start && start > 60)
                        bitchain->byte = obj->address + obj->size - 2;
                    }
                  if (!bit_check_CRC (bitchain, obj->address, 0xC0C1))
                    error |= DWG_ERR_WRONGCRC;
                }
              }
            }
          // add to block header
          if (_hdr && obj->supertype == DWG_SUPERTYPE_ENTITY
              && obj->fixedtype != DWG_TYPE_UNUSED
              && obj->fixedtype != DWG_TYPE_JUMP
              && obj->type != DWG_TYPE_VERTEX_r11
              && obj->fixedtype != DWG_TYPE_SEQEND)
            {
              BITCODE_H ref;
              if (!obj->handle.value)
                obj->handle.value = dwg_next_handle (dwg);
              hdr = &dwg->object[hdr_index];
              ref = dwg_add_handleref (dwg, 3, obj->handle.value, hdr);
              // if (dwg->dirty_refs)
              // find _hdr again from hdr_handle
              LOG_TRACE ("BLOCK_HEADER \"%s\".", _hdr->name);
              if (obj->fixedtype != DWG_TYPE_BLOCK)
                PUSH_HV (_hdr, num_owned, entities, ref);
              obj->tio.entity->ownerhandle
                  = dwg_add_handleref (dwg, 4, hdr_handle, obj);
              obj->tio.entity->ownerhandle->r11_idx = block_idx;
              LOG_TRACE ("ownerhandle: " FORMAT_HREF11 "\n",
                         ARGS_HREF11 (obj->tio.entity->ownerhandle));
            }
          num++;
          if (bitchain->byte < oldpos + size)
            LOG_TRACE ("\n");
          if (bitchain->byte >= bitchain->size && (BITCODE_RL)bitchain->byte != end)
            {
              LOG_ERROR ("Too many entities, buffer overflow %" PRIuSIZE
                         " >= %" PRIuSIZE,
                         bitchain->byte, bitchain->size);
              return DWG_ERR_INVALIDDWG;
            }
        }
      if ((BITCODE_RL)bitchain->byte != end)
        {
          LOG_ERROR ("@0x%zx => end 0x%x", bitchain->byte, end);
          return DWG_ERR_INVALIDDWG;
        }
    }

  SINCE (R_11)
  {
    switch (entity_section)
      {
      case ENTITIES_SECTION_INDEX:
        DECODE_PRER13_SENTINEL (DWG_SENTINEL_R11_ENTITIES_END);
        break;
      case BLOCKS_SECTION_INDEX:
        DECODE_PRER13_SENTINEL (DWG_SENTINEL_R11_BLOCK_ENTITIES_END);
        break;
      case EXTRAS_SECTION_INDEX:
        DECODE_PRER13_SENTINEL (DWG_SENTINEL_R11_EXTRA_ENTITIES_END);
        break;
      default:
        LOG_ERROR ("Internal error: Illegal entity_section %d 0-2\n",
                   (int)entity_section);
        return DWG_ERR_INTERNALERROR;
      }
  }

  LOG_INFO ("==========================================\n");
  LOG_TRACE ("%s: end\n", entities_section[entity_section]);

  return error;
}