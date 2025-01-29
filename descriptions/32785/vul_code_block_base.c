{
                      const Dwg_DYNAPI_field *num_f;
                      // FIELD_2RD_VECTOR (clip_verts, num_clip_verts, 11|14);
                      if (pair->code >= 10 && pair->code <= 24
                          && strEQc (f->name, "clip_verts")) // 11 or 14
                        {
                          BITCODE_BL num_clip_verts = 0;
                          BITCODE_2RD *clip_verts;
                          // 11 has no num_clip_verts: realloc. clip_inverts
                          // has.
                          if (pair->code == 14 || pair->code == 24)
                            {
                              dwg_dynapi_entity_value (_obj, obj->name,
                                                       "num_clip_verts",
                                                       &num_clip_verts, NULL);
                              LOG_INSANE ("%s.num_clip_verts = %d, j = %d\n",
                                          name, num_clip_verts, j);
                            }
                          dwg_dynapi_entity_value (_obj, obj->name,
                                                   "clip_verts", &clip_verts,
                                                   NULL);
                          // assert (j == 0 || j < (int)num_clip_verts);
                          if (pair->code < 20)
                            {
                              // no need to realloc
                              if (!j && pair->code == 14)
                                {
                                  clip_verts = (BITCODE_2RD *)xcalloc (
                                      num_clip_verts, sizeof (BITCODE_2RD));
                                  dwg_dynapi_entity_set_value (_obj, obj->name,
                                                               f->name,
                                                               &clip_verts, 0);
                                }
                              else if (pair->code == 11)
                                {
                                  clip_verts = (BITCODE_2RD *)realloc (
                                      clip_verts,
                                      (j + 1) * sizeof (BITCODE_2RD));
                                  memset (&clip_verts[j], 0,
                                          sizeof (BITCODE_2RD));
                                  dwg_dynapi_entity_set_value (_obj, obj->name,
                                                               f->name,
                                                               &clip_verts, 0);
                                }
                              if (j >= 0 && j < (int)num_clip_verts
                                  && clip_verts)
                                clip_verts[j].x = pair->value.d;
                            }
                          else if (pair->code < 30)
                            {
                              if (j >= 0 && j < (int)num_clip_verts
                                  && clip_verts)
                                {
                                  clip_verts[j].y = pair->value.d;
                                  LOG_TRACE (
                                      "%s.%s[%d] = (%f, %f) [2RD* %d]\n", name,
                                      "clip_verts", j, clip_verts[j].x,
                                      clip_verts[j].y, pair->code - 10);
                                  j++;
                                }
                              if (pair->code == 21)
                                {
                                  dwg_dynapi_entity_set_value (
                                      _obj, obj->name, "num_clip_verts", &j,
                                      0);
                                  LOG_TRACE ("%s.num_clip_verts = %d\n", name,
                                             j);
                                }
                            }
                          goto next_pair;
                        }
                      // point vectors with known num_field
                      else if ((*f->type == '2' || *f->type == '3')
                               && (f->type[2] == 'D'
                                   || strEQc (&f->type[1], "DPOINT*"))
                               && (num_f = find_numfield (fields, f->name)))
                        {
                          long size = get_numfield_value (
                              _obj, num_f); // how many points
                          double *pts;
                          int is2d = *f->type == '2';
                          if (!size)
                            {
                              LOG_TRACE ("Ignore empty %s.%s VECTOR [%s %d]\n",
                                         name, f->name, f->type, pair->code);
                              goto next_pair;
                            }
                          else if (j == 0 && pair->code < 20)
                            {
                              pts = (double *)xcalloc (size, is2d ? 16 : 24);
                              if (!pts)
                                return NULL;
                              LOG_TRACE ("%s.%s size: %ld\n", name, f->name,
                                         size);
                              pts[0] = pair->value.d;
                              dwg_dynapi_entity_set_value (_obj, obj->name,
                                                           f->name, &pts, 0);
                            }
                          else if (j < size)
                            {
                              int _i = is2d ? j * 2 : j * 3;
                              dwg_dynapi_entity_value (_obj, obj->name,
                                                       f->name, &pts, NULL);
                              if (pair->code < 20 && pts)
                                {
                                  pts[_i] = pair->value.d;
                                }
                              else if (pair->code < 30 && pts)
                                {
                                  if (is2d)
                                    LOG_TRACE (
                                        "%s.%s[%d] = (%f, %f) [%s %d]\n", name,
                                        f->name, j, pts[_i], pair->value.d,
                                        f->type, pair->code);
                                  pts[_i + 1] = pair->value.d;
                                }
                              else if (*f->type == '3' && pts)
                                {
                                  LOG_TRACE (
                                      "%s.%s[%d] = (%f, %f, %f) [%s %d]\n",
                                      name, f->name, j, pts[_i], pts[_i + 1],
                                      pair->value.d, f->type, pair->code);
                                  pts[_i + 2] = pair->value.d;
                                  if (j == size - 1)
                                    j = 0; // restart
                                }
                            }
                          else if (j > size)
                            LOG_ERROR ("%s.%s overflow %d > %ld", name,
                                       num_f->name, j, size)
                        }
                      else if (f->dxf == pair->code)
                        {
                          LOG_WARN ("Ignore %s.%s VECTOR [%s %d]", name,
                                    f->name, f->type, pair->code);
                          goto next_pair;
                        }
                    }
                  else if (obj->fixedtype == DWG_TYPE_VISUALSTYLE
                           && dwg->header.from_version >= R_2010 && pair->code == 176
                           && prev_vstyle)
                    {
                      // which 176 of the many? the one after the previous
                      // field
                      char fieldname[40];
                      strcpy (fieldname, prev_vstyle->name);
                      strcat (fieldname, "_int");
                      f = prev_vstyle + 1;
                      if (strEQc (fieldname, "display_brightness_bl_int"))
                        {
                          strcpy (fieldname, "display_brightness_int");
                          f++;
                        }
                      if (strEQ (fieldname, f->name))
                        {
                          LOG_HANDLE ("found %s.%s:\n", name, fieldname);
                          if (strEQc (f->name, "display_shadow_type_int")
                              && dwg->header.from_version >= R_2013)
                            {
                              Dwg_Object_VISUALSTYLE *o = obj->tio.object->tio.VISUALSTYLE;
                              o->display_shadow_type_int = pair->value.i;
                              pair = add_VISUALSTYLE_props (obj, dat);
                              if (!pair) // success
                                goto start_loop;
                              else // or better advance to the next 0
                                goto search_field;
                            }
                          else
                            goto matching_pair;
                        }
                      else
                        LOG_WARN ("%s.%s [BS 176] not found in dynapi", name,
                                  fieldname);
                    }
                  else if (f->dxf == pair->code) // matching DXF code
                    {
                    matching_pair:
                      if (obj->fixedtype == DWG_TYPE_VISUALSTYLE
                          && dwg->header.from_version >= R_2010 && pair->code != 176)
                        {
                          prev_vstyle = f;
                        }
                      // exceptions, where there's another field 92:
                      if (pair->code == 92 && is_entity
                          && dwg->header.from_version < R_2010
                          && strEQc (subclass, "AcDbEntity"))
                        // not MULTILEADER.text_color, nor MESH.num_vertex
                        {
                          pair = add_ent_preview (obj, dat, pair);
                          goto start_loop;
                        }
                      else if (pair->code == 3 && pair->value.s
                               && memBEGINc (obj->name, "DICTIONARY")
                               && strNE (obj->name, "DICTIONARYVAR"))
                        {
                          strncpy (text, pair->value.s, 254);
                          text[255] = '\0';
                          goto next_pair; // skip setting texts TV*
                        }
                      // convert angle to radians
                      else if (pair->code >= 50 && pair->code <= 55)
                        {
                          BITCODE_BD ang;
                          if (pair->value.d == 0.0)
                            goto next_pair;
                          ang = deg2rad (pair->value.d);
                          dwg_dynapi_entity_set_value (_obj, obj->name,
                                                       f->name, &ang, 0);
                          LOG_TRACE ("%s.%s = %f (from DEG %f°) [%s %d]\n",
                                     name, f->name, ang, pair->value.d,
                                     f->type, pair->code);
                          goto next_pair; // found
                        }
                      // convert double to text (e.g. ATEXT)
                      else if (strEQc (f->type, "D2T")
                               && pair->type == DWG_VT_REAL)
                        {
                          // TODO: for now we need to do double-conversion
                          // (str->dbl->str), because we don't have the initial
                          // dat->byte position.
                          char *d2t = (char *)xcalloc (36, 1);
                          sprintf (d2t, "%f", pair->value.d);
                          dwg_dynapi_entity_set_value (_obj, obj->name,
                                                       f->name, &d2t, 1);
                          LOG_TRACE ("%s.%s = %s (from %f°) [%s %d]\n", name,
                                     f->name, d2t, pair->value.d, f->type,
                                     pair->code);
                          free (d2t);
                          goto next_pair; // found
                        }
                      // resolve handle, by name or ref
                      else if (strEQc (f->type, "H"))
                        {
                          BITCODE_H ref = find_tablehandle (dwg, pair);
                          if (!ref)
                            {
                              if (pair->code > 300)
                                {
                                  int code = 5; // default: soft pointer
                                  if (obj->fixedtype == DWG_TYPE_VIEWPORT)
                                    {
                                      switch (pair->code)
                                        {
                                        case 340:
                                        case 332:
                                        case 333:
                                          code = 4;
                                          break;
                                        case 361:
                                          code = 3;
                                          break;
                                        default:
                                          break;
                                        }
                                    }
                                  else if (strEQc (f->name, "history_id"))
                                    code = 4;
                                  else if (strEQc (f->name, "background"))
                                    code = 4;
                                  else if (strEQc (f->name, "dimensionobj"))
                                    code = 4;
                                  else if (strEQc (f->name, "active_viewport")
                                           || strEQc (f->name, "host_block"))
                                    code = 4;
                                  else if (strEQc (f->name, "writedep")
                                           || strEQc (f->name, "readdep"))
                                    code = 4;
                                  else if (strEQc (f->name, "imagedefreactor"))
                                    code = 3;
                                  else if (strEQc (f->name, "table_style"))
                                    code = 3;
                                  ref = dwg_add_handleref (dwg, code,
                                                           pair->value.u, obj);
                                  LOG_TRACE ("%s.%s = " FORMAT_REF " [H %d]\n",
                                             name, f->name, ARGS_REF (ref),
                                             pair->code);
                                }
                              else if (pair->type == DWG_VT_INT32 && pair->value.u)
                                {
                                  ref = dwg_add_handleref (dwg, 5,
                                                           pair->value.u, obj);
                                  LOG_TRACE ("%s.%s = " FORMAT_REF " [H %d]\n",
                                             name, f->name, ARGS_REF (ref),
                                             pair->code);
                                }
                              else if ((pair->type == DWG_VT_STRING
                                        || pair->type == DWG_VT_HANDLE)
                                       && pair->value.s)
                                {
                                  obj_hdls = array_push (
                                      obj_hdls, f->name, pair->value.s,
                                      obj->tio.object->objid);
                                  LOG_TRACE ("%s.%s: name %s -> H for code "
                                             "%d later\n",
                                             name, f->name, pair->value.s,
                                             pair->code);
                                }
                            }
                          else
                            {
                              dwg_dynapi_entity_set_value (_obj, obj->name,
                                                           f->name, &ref, 1);
                              LOG_TRACE ("%s.%s = " FORMAT_REF " [H %d]\n",
                                         name, f->name, ARGS_REF (ref),
                                         pair->code);
                            }
                          goto next_pair; // found
                        }
                      // only 2D or 3D points .x
                      else if (f->size > 8
                               && (strchr (f->type, '2')
                                   || strchr (f->type, '3')
                                   || strEQc (f->type, "BE")))
                        {
                          // pt.x = 0.0;
                          // if (pair->value.d == 0.0) // ignore defaults
                          //  goto next_pair;
                          pt.x = pair->value.d;
                          dwg_dynapi_entity_set_value (_obj, obj->name,
                                                       f->name, &pt, 1);
                          LOG_TRACE ("%s.%s.x = %f [%s %d]\n", name, f->name,
                                     pair->value.d, f->type, pair->code);
                          goto next_pair; // found
                        }
                      else if (pair->type == DWG_VT_REAL
                               && strEQc (f->type, "TIMEBLL"))
                        {
                          static BITCODE_TIMEBLL date = { 0, 0, 0 };
                          date.value = pair->value.d;
                          date.days = (BITCODE_BL)trunc (pair->value.d);
                          date.ms = (BITCODE_BL) (86400000.0
                                                  * (date.value - date.days));
                          LOG_TRACE ("%s.%s %.09f (" FORMAT_BL ", " FORMAT_BL
                                     ") [TIMEBLL %d]\n",
                                     name, f->name, date.value, date.days,
                                     date.ms, pair->code);
                          dwg_dynapi_entity_set_value (_obj, obj->name,
                                                       f->name, &date, 1);
                          goto next_pair;
                        }
                      else if (f->size > 8 && strEQc (f->type, "CMC"))
                        {
                          BITCODE_CMC color;
                          dwg_dynapi_entity_value (_obj, obj->name, f->name,
                                                   &color, NULL);
                          if (pair->code < 90)
                            {
                              color.index = pair->value.i;
                              if (pair->value.i == 256)
                                color.method = 0xc2;
                              else if (pair->value.i == 257)
                                color.method = 0xc8;
                              else if (pair->value.i < 256 && dat->from_version >= R_2004)
                                {
                                  color.method = 0xc3;
                                  color.rgb = 0xc3000000 | color.index;
                                  color.index = 256;
                                }
                              LOG_TRACE ("%s.%s.index = %d [%s %d]\n", name,
                                         f->name, color.index, "CMC",
                                         pair->code);
                              if (color.rgb)
                                LOG_TRACE ("%s.%s.rgb = 0x%08x [%s %d]\n", name,
                                           f->name, color.rgb, "CMC",
                                           pair->code);
                            }
                          else if (pair->code < 430)
                            {
                              color.rgb = pair->value.l;
                              color.method = pair->value.l >> 0x18;
                              if (pair->value.l == 257)
                                {
                                  color.method = 0xc8;
                                  color.rgb = 0xc8000000;
                                }
                              // color.alpha = (pair->value.l & 0xFF000000) >>
                              // 24; if (color.alpha)
                              //  color.alpha_type = 3;
                              LOG_TRACE ("%s.%s.rgb = %08X [%s %d]\n", name,
                                         f->name, pair->value.u, "CMC",
                                         pair->code);
                            }
                          else if (pair->code < 440)
                            {
                              color.flag |= 0x10;
                              color.name = dwg_add_u8_input (dwg, pair->value.s);
                              LOG_TRACE ("%s.%s.name = %s [%s %d]\n", name,
                                         f->name, pair->value.s, "CMC",
                                         pair->code);
                            }
                          else if (pair->code < 450)
                            {
                              color.alpha = (pair->value.l & 0xFF000000) >> 24;
                              if (color.alpha)
                                color.alpha_type = 3;
                              LOG_TRACE ("%s.%s.alpha = %08X [%s %d]\n", name,
                                         f->name, pair->value.u, "CMC",
                                         pair->code);
                            }
                          dwg_dynapi_entity_set_value (_obj, obj->name,
                                                       f->name, &color, 1);
                          goto next_pair; // found, early exit
                        }
                      else
                        dwg_dynapi_entity_set_value (_obj, obj->name, f->name,
                                                     &pair->value, 1);
                      if (f->is_string)
                        {
                          LOG_TRACE ("%s.%s = %s [%s %d]\n", name, f->name,
                                     pair->value.s, f->type, pair->code);
                        }
                      else if (strchr (&f->type[1], 'D'))
                        {
                          LOG_TRACE ("%s.%s = %f [%s %d]\n", name, f->name,
                                     pair->value.d, f->type, pair->code);
                        }
                      else
                        {
                          LOG_TRACE ("%s.%s = %ld [%s %d]\n", name, f->name,
                                     pair->value.l, f->type, pair->code);
                        }
                      goto next_pair; // found, early exit
                    }
                  // wrong code, maybe a point .y or .z
                  else if ((*f->type == '3' || *f->type == '2'
                            || strEQc (f->type, "BE"))
                           && (strstr (f->type, "_1")
                                   ? f->dxf + 1 == pair->code // 2BD_1
                                   : f->dxf + 10 == pair->code))
                    {
                      // pt.y = 0.0;
                      // if (pair->value.d == 0.0) // ignore defaults
                      //  goto next_pair;
                      dwg_dynapi_entity_value (_obj, obj->name, f->name, &pt,
                                               NULL);
                      pt.y = pair->value.d;
                      dwg_dynapi_entity_set_value (_obj, obj->name, f->name,
                                                   &pt, 1);
                      LOG_TRACE ("%s.%s.y = %f [%s %d]\n", name, f->name,
                                 pair->value.d, f->type, pair->code);
                      goto next_pair; // found, early exit
                    }
                  else if ((*f->type == '3' || *f->type == '2'
                            || strEQc (f->type, "BE"))
                           && (strstr (f->type, "_1")
                                   ? f->dxf + 2 == pair->code // 2BD_1
                                   : f->dxf + 20 == pair->code))
                    {
                      pt.z = 0.0;
                      // can ignore z or 0.0? e.g. no VPORT.view_target
                      if (strNE (name, "_3DFACE") && strNE (f->name, "scale")
                          && *f->type == '2')
                        goto next_pair;
                      dwg_dynapi_entity_value (_obj, obj->name, f->name, &pt,
                                               NULL);
                      pt.z = pair->value.d;
                      dwg_dynapi_entity_set_value (_obj, obj->name, f->name,
                                                   &pt, 0);
                      LOG_TRACE ("%s.%s.z = %f [%s %d]\n", name, f->name,
                                 pair->value.d, f->type, pair->code);

                      // 3DD scale
                      if (strEQc (f->name, "scale")
                          && dwg->header.version >= R_2000
                          && dwg_dynapi_entity_field (obj->name, "scale_flag")
                          && dwg_dynapi_entity_value (_obj, obj->name,
                                                      "scale_flag",
                                                      &scale_flag, NULL))
                        { // set scale_flag
                          scale_flag = 0;
                          if (pt.x == 1.0 && pt.y == 1.0 && pt.z == 1.0)
                            scale_flag = 3;
                          else if (pt.x == 1.0)
                            scale_flag = 1;
                          else if (pt.x == pt.y && pt.x == pt.z)
                            scale_flag = 2;
                          dwg_dynapi_entity_set_value (
                              _obj, obj->name, "scale_flag", &scale_flag, 0);
                          LOG_TRACE ("%s.scale_flag = %d [BB 0]\n", name,
                                     scale_flag);
                        }
                      // 3DFACE.z_is_zero
                      else if (strEQc (name, "_3DFACE")
                               && strEQc (f->name, "corner1")
                               && dwg->header.version >= R_2000 && pt.z == 0.0)
                        {
                          BITCODE_B z_is_zero = 1;
                          dwg_dynapi_entity_set_value (
                              _obj, obj->name, "z_is_zero", &z_is_zero, 0);
                          LOG_TRACE ("%s.z_is_zero = 1 [B 0]\n", name);
                        }

                      goto next_pair; // found, early exit
                    }
                  // FIELD_VECTOR_N BITCODE_BD transmatrix[16]:
                  else if (strEQc (f->type, "BD*")
                           && (strEQc (name, "EXTRUDEDSURFACE")
                               || strEQc (name, "LOFTEDSURFACE")
                               || strEQc (name, "SWEPTSURFACE")
                               || strEQc (name, "REVOLVEDSURFACE")
                               || strEQc (name, "MATERIAL")
                               || strEQc (name, "SPATIAL_FILTER")
                               || /* max 12 */
                               strEQc (name, "ACSH_SWEEP_CLASS"))
                           && ((pair->code >= 40 && pair->code <= 49)
                               || (pair->code <= 142 && pair->code <= 147)))
                    {
                      // 16x BD, via j
                      BITCODE_BD *matrix;
                      dwg_dynapi_entity_value (_obj, obj->name, f->name,
                                               &matrix, NULL);
                      if (!matrix)
                        {
                          matrix = (BITCODE_BD *)xcalloc (16,
                                                          sizeof (BITCODE_BD));
                          if (!matrix)
                            goto invalid_dxf;
                          j = 0;
                        }
                      if (j < 0 || j >= 16 || !matrix)
                        goto invalid_dxf;
                      assert (j >= 0 && j < 16 && matrix);
                      matrix[j] = pair->value.d;
                      dwg_dynapi_entity_set_value (_obj, obj->name, f->name,
                                                   &matrix, 0);
                      LOG_TRACE ("%s.%s[%d] = %f [%s %d]\n", name, f->name, j,
                                 pair->value.d, f->type, pair->code);
                      j++;
                      goto next_pair;
                    }