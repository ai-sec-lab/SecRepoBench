const Dwg_DYNAPI_field *f1;
    BITCODE_B is_xref_ref = 1;
    // set defaults not in dxf:
    if (dwg_dynapi_entity_field (obj->name, "is_xref_ref"))
      dwg_dynapi_entity_set_value (_obj, obj->name, "is_xref_ref",
                                   &is_xref_ref, 0);
    if ((f1 = dwg_dynapi_entity_field (obj->name, "scale_flag"))
        && (memBEGINc(f1->type, "BB")))
      {
        scale_flag = 3;
        dwg_dynapi_entity_set_value (_obj, obj->name, "scale_flag",
                                     &scale_flag, 0);
        LOG_TRACE ("%s.scale_flag = 3 (default)\n", obj->name);
      }
    if ((f1 = dwg_dynapi_entity_field (obj->name, "width_factor"))
        && (memBEGINc(f1->type, "RD") || memBEGINc(f1->type, "BD")))
      {
        BITCODE_BD width_factor = 1.0;
        dwg_dynapi_entity_set_value (_obj, obj->name, "width_factor",
                                     &width_factor, 0);
        LOG_TRACE ("%s.width_factor = 1.0 (default)\n", obj->name);
      }
    if ((f1 = dwg_dynapi_entity_field (obj->name, "scale"))
        && (memBEGINc(f1->type, "3BD")))
      {
        pt.x = pt.y = pt.z = 1.0;
        dwg_dynapi_entity_set_value (_obj, obj->name, "scale", &pt, 0);
        LOG_TRACE ("%s.scale = (1,1,1) (default)\n", obj->name);
        pt.x = pt.y = pt.z = 0.0;
      }
    if ((f1 = dwg_dynapi_entity_field (obj->name, "extrusion"))
        && (memBEGINc(f1->type, "BE") || memBEGINc(f1->type, "3BD")))
      {
        pt.x = pt.y = 0.0;
        pt.z = 1.0;
        dwg_dynapi_entity_set_value (_obj, obj->name, "extrusion", &pt, 0);
        LOG_TRACE ("%s.extrusion = (0,0,1) (default)\n", obj->name);
        pt.z = 0.0;
      }