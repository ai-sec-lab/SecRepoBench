Dwg_Object *obj;                                                            \
  Dwg_Object_##token *_obj;                                                   \
  Dwg_Object *ctrl = dwg_get_first_object (dwg, DWG_TYPE_##token##_CONTROL);  \
  Dwg_Object_##token##_CONTROL *_ctrl                                         \
    = ctrl ? ctrl->tio.object->tio.token##_CONTROL : NULL;                    \
  if (!ctrl || dat->byte > dat->size || (num + i) > dwg->num_objects)         \
    return DWG_ERR_INVALIDDWG;                                                \
  flag = bit_read_RC (dat);                                                   \
  name = bit_read_TF (dat, 32);                                               \
  _obj = dwg_add_##token (dwg, (const char *)name);                           \
  obj = dwg_obj_generic_to_object (_obj, &error);                             \
  _ctrl->entries[i] = dwg_add_handleref (dwg, 2, obj->handle.value, obj);     \
  obj->size = tbl->size;                                                      \
  obj->address = pos;                                                         \
  _obj->flag = flag;                                                          \
  LOG_TRACE ("\n-- table entry " #token " [%d]: 0x%lx\n", i, pos);            \
  LOG_TRACE ("flag: %u [RC 70]\n", flag);                                     \
  LOG_TRACE ("name: \"%s\" [TF 32 2]\n", name);                               \
  free (name)