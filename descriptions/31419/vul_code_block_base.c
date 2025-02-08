
  dat->opts |= DWG_OPTS_INDXF;
  dwg->opts |= DWG_OPTS_INDXF;
  //num_dxf_objs = 0;
  //size_dxf_objs = 1000;
  //dxf_objs = (Dxf_Objs *)malloc (1000 * sizeof (Dxf_Objs));
  if (!dwg->object_map)
    dwg->object_map = hash_new (dat->size / 1000);
  // cannot rely on ref->obj during realloc's
  dwg->dirty_refs = 1;

  header_hdls = new_array_hdls (16);
  eed_hdls = new_array_hdls (16);
  obj_hdls = new_array_hdls (16);

  // start with the BLOCK_HEADER at objid 0
  if (!dwg->num_objects)
    {
      Dwg_Object *obj;
      Dwg_Object_BLOCK_HEADER *_obj;
      char *dxfname = strdup ((char *)"BLOCK_HEADER");
      NEW_OBJECT (dwg, obj);
      ADD_OBJECT (BLOCK_HEADER);
      // dwg->header.version probably here still unknown. <r2000: 0x17
      // later fixed up when reading $ACADVER and the BLOCK_HEADER.name
      _obj->name = dwg_add_u8_input (dwg, "*Model_Space");
      _obj->is_xref_ref = 1;
      obj->tio.object->is_xdic_missing = 1;
      dwg_add_handle (&obj->handle, 0, 0x1F, obj);
      obj->tio.object->ownerhandle = dwg_add_handleref (dwg, 4, 1, NULL);
    }

  while (dat->byte < dat->size)
    {
      pair = dxf_read_pair (dat);
      DXF_BREAK_EOF;
      pair = dxf_expect_code (dat, pair, 0);
      DXF_BREAK_EOF;
      if (pair->type == DWG_VT_STRING && strEQc (pair->value.s, "SECTION"))
        {
          dxf_free_pair (pair);
          pair = dxf_read_pair (dat);
          DXF_BREAK_EOF;
          pair = dxf_expect_code (dat, pair, 2);
          DXF_BREAK_EOF;
          if (!pair->value.s)
            {
              LOG_ERROR ("Expected SECTION string code 2, got code %d",
                         pair->code);
              dxf_free_pair (pair);
              pair = NULL;
              break;
            }
          else if (strEQc (pair->value.s, "HEADER"))
            {
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_header_read (dat, dwg);
              if (error > DWG_ERR_CRITICAL)
                goto error;
              dxf_fixup_header (dwg);
              // skip minimal DXF
              /*
              if (!dwg->header_vars.DIMPOST) // T in all versions
                {
                  LOG_ERROR ("Unsupported minimal DXF");
                  return DWG_ERR_INVALIDDWG;
                }
              */
            }
          else if (strEQc (pair->value.s, "CLASSES"))
            {
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_classes_read (dat, dwg);
              if (error > DWG_ERR_CRITICAL)
                return error;
            }
          else if (strEQc (pair->value.s, "TABLES"))
            {
              BITCODE_H hdl;
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_tables_read (dat, dwg);
              if (error > DWG_ERR_CRITICAL)
                goto error;

              resolve_postponed_header_refs (dwg);
              resolve_postponed_eed_refs (dwg);

              // should not happen
              if (!dwg->header_vars.LTYPE_BYLAYER
                  && (hdl = dwg_find_tablehandle_silent (
                          dwg, (char *)"ByLayer", "LTYPE")))
                dwg->header_vars.LTYPE_BYLAYER
                    = dwg_add_handleref (dwg, 5, hdl->handleref.value, NULL);
              // should not happen
              if (!dwg->header_vars.LTYPE_BYBLOCK
                  && (hdl = dwg_find_tablehandle_silent (
                          dwg, (char *)"ByBlock", "LTYPE")))
                dwg->header_vars.LTYPE_BYBLOCK
                    = dwg_add_handleref (dwg, 5, hdl->handleref.value, NULL);
              // but this is needed
              if (!dwg->header_vars.LTYPE_CONTINUOUS
                  && (hdl = dwg_find_tablehandle_silent (
                          dwg, (char *)"Continuous", "LTYPE")))
                dwg->header_vars.LTYPE_CONTINUOUS
                    = dwg_add_handleref (dwg, 5, hdl->handleref.value, NULL);
            }
          else if (strEQc (pair->value.s, "BLOCKS"))
            {
              BITCODE_H hdl;
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_blocks_read (dat, dwg);
              if (error > DWG_ERR_CRITICAL)
                goto error;

              // resolve_postponed_header_refs (dwg);
              if (!dwg->header_vars.BLOCK_RECORD_PSPACE
                  && (hdl = dwg_find_tablehandle_silent (
                          dwg, (char *)"*Paper_Space", "BLOCK")))
                dwg->header_vars.BLOCK_RECORD_PSPACE
                    = dwg_add_handleref (dwg, 5, hdl->handleref.value, NULL);
              if (!dwg->header_vars.BLOCK_RECORD_MSPACE
                  && (hdl = dwg_find_tablehandle_silent (
                          dwg, (char *)"*Model_Space", "BLOCK")))
                dwg->header_vars.BLOCK_RECORD_MSPACE
                    = dwg_add_handleref (dwg, 5, hdl->handleref.value, NULL);
            }
          else if (strEQc (pair->value.s, "ENTITIES"))
            {
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_entities_read (dat, dwg);
              if (error > DWG_ERR_CRITICAL)
                goto error;
            }
          else if (strEQc (pair->value.s, "OBJECTS"))
            {
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_objects_read (dat, dwg);
              if (error > DWG_ERR_CRITICAL)
                goto error;
              resolve_header_dicts (dwg);
            }
          else if (strEQc (pair->value.s, "THUMBNAILIMAGE"))
            {
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_thumbnail_read (dat, dwg);
            }
          else if (strEQc (pair->value.s, "ACDSDATA"))
            {
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_acds_read (dat, dwg);
            }
          else // if (strEQc (pair->value.s, ""))
            {
              LOG_WARN ("SECTION %s ignored for now", pair->value.s);
              dxf_free_pair (pair);
              pair = NULL;
              error = dxf_unknownsection_read (dat, dwg);
            }
        }
    }
  if (pair != NULL && pair->code == 0 &&
      pair->value.s != NULL && strEQc (pair->value.s, "EOF"))
    ;
  else if (dat->byte >= dat->size || (pair == NULL))
    error |= DWG_ERR_IOERROR;
  resolve_postponed_header_refs (dwg);
  resolve_postponed_object_refs (dwg);
  LOG_HANDLE ("Resolving pointers from ObjectRef vector:\n");
  dwg_resolve_objectrefs_silent (dwg);
  free_array_hdls (header_hdls);
  free_array_hdls (eed_hdls);
  free_array_hdls (obj_hdls);
  if (dwg->header.version <= R_2000 && dwg->header.from_version > R_2000)
    dwg_fixup_BLOCKS_entities (dwg);
  LOG_TRACE ("import from DXF\n");
  if (error > DWG_ERR_CRITICAL)
    return error;
  else
    return dwg->num_objects ? 1 : 0;

error:
  dwg->dirty_refs = 0;
  free_array_hdls (header_hdls);
  free_array_hdls (eed_hdls);
  free_array_hdls (obj_hdls);
  return error;