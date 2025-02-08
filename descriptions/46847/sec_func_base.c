static Dxf_Pair *
add_TABLEGEOMETRY_Cell (Dwg_Object *restrict obj, Bit_Chain *restrict dat,
                        Dxf_Pair *restrict pair)
{
  Dwg_Data *dwg = obj->parent;
  Dwg_Object_TABLEGEOMETRY *o = obj->tio.object->tio.TABLEGEOMETRY;
  BITCODE_H hdl;
  BITCODE_BL num_cells = o->num_cells;
  int i = -1, j = -1;
	
  if (num_cells < 1)
    {
      return NULL;
    }

  o->cells = (Dwg_TABLEGEOMETRY_Cell *)xcalloc (
      num_cells, sizeof (Dwg_TABLEGEOMETRY_Cell));
  if (!o->cells)
    {
      o->num_cells = 0;
      return NULL;
    }

  while (pair != NULL && pair->code != 0)
    {
      switch (pair->code)
        {
        case 0:
          break;
        case 93:
          i++; // the first
#define CHK_cells                                                             \
  if (i < 0 || i >= (int)num_cells || !o->cells)                              \
    return NULL;                                                              \
  assert (i >= 0 && i < (int)num_cells);                                      \
  assert (o->cells)

          CHK_cells;
          o->cells[i].geom_data_flag = pair->value.i;
          LOG_TRACE ("%s.cells[%d].geom_data_flag = " FORMAT_BL " [BL %d]\n",
                     obj->name, i, o->cells[i].geom_data_flag, pair->code);
          break;
        case 40:
          CHK_cells;
          o->cells[i].width_w_gap = pair->value.d;
          LOG_TRACE ("%s.cells[%d].width_w_gap = %f [BD %d]\n", obj->name, i,
                     pair->value.d, pair->code);
          break;
        case 41:
          CHK_cells;
          o->cells[i].height_w_gap = pair->value.d;
          LOG_TRACE ("%s.cells[%d].height_w_gap = %f [BD %d]\n", obj->name, i,
                     pair->value.d, pair->code);
          break;
        case 330:
          CHK_cells;
          hdl = find_tablehandle (dwg, pair);
          if (hdl)
            {
              if (hdl->handleref.code != 4) // turn the 5 into a 4
                o->cells[i].tablegeometry
                    = dwg_add_handleref (dwg, 4, hdl->handleref.value, NULL);
              else
                o->cells[i].tablegeometry = hdl;
            }
          else
            o->cells[i].tablegeometry = dwg_add_handleref (dwg, 4, 0, NULL);
          LOG_TRACE ("%s.cells[%d].tablegeometry = " FORMAT_REF " [H %d]\n",
                     obj->name, i, ARGS_REF (o->cells[i].tablegeometry),
                     pair->code);
          break;
        case 94:
          CHK_cells;
          o->cells[i].num_geometry = pair->value.i;
          LOG_TRACE ("%s.cells[%d].num_geometry = " FORMAT_BL " [BL %d]\n",
                     obj->name, i, o->cells[i].num_geometry, pair->code);
          o->cells[i].geometry = (Dwg_CellContentGeometry *)xcalloc (
              pair->value.i, sizeof (Dwg_CellContentGeometry));
          if (!o->cells[i].geometry)
            {
              o->cells[i].num_geometry = 0;
              return NULL;
            }
          j = -1;
          break;
        case 10:
          CHK_cells;
          j++;

#define CHK_geometry                                                          \
  if (j < 0 || j >= (int)o->cells[i].num_geometry || !o->cells[i].geometry)   \
    return NULL;                                                              \
  assert (j >= 0 && j < (int)o->cells[i].num_geometry);                       \
  assert (o->cells[i].geometry)

          CHK_geometry;
          o->cells[i].geometry[j].dist_top_left.x = pair->value.d;
          LOG_TRACE (
              "%s.cells[%d].geometry[%d].dist_top_left.x = %f [BD %d]\n",
              obj->name, i, j, pair->value.d, pair->code);
          break;
        case 20:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].dist_top_left.y = pair->value.d;
          break;
        case 30:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].dist_top_left.z = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].dist_top_left = ( %f, %f, %f) "
                     "[3BD 10]\n",
                     obj->name, i, j, o->cells[i].geometry[j].dist_top_left.x,
                     o->cells[i].geometry[j].dist_top_left.y, pair->value.d);
          break;
        case 11:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].dist_center.x = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].dist_center.x = %f [BD %d]\n",
                     obj->name, i, j, pair->value.d, pair->code);
          break;
        case 21:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].dist_center.y = pair->value.d;
          break;
        case 31:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].dist_center.z = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].dist_center = ( %f, %f, %f) "
                     "[3BD 10]\n",
                     obj->name, i, j, o->cells[i].geometry[j].dist_center.x,
                     o->cells[i].geometry[j].dist_center.y, pair->value.d);
          break;
        case 43:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].content_width = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].content_width = %f [BD %d]\n",
                     obj->name, i, j, pair->value.d, pair->code);
          break;
        case 44:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].content_height = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].content_height = %f [BD %d]\n",
                     obj->name, i, j, pair->value.d, pair->code);
          break;
        case 45:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].width = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].width = %f [BD %d]\n",
                     obj->name, i, j, pair->value.d, pair->code);
          break;
        case 46:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].height = pair->value.d;
          LOG_TRACE ("%s.cells[%d].geometry[%d].height = %f [BD %d]\n",
                     obj->name, i, j, pair->value.d, pair->code);
          break;
        case 95:
          CHK_cells;
          CHK_geometry;
          o->cells[i].geometry[j].unknown = pair->value.i;
          LOG_TRACE ("%s.cells[%d].geometry[%d].unknown = %d [BL %d]\n",
                     obj->name, i, j, pair->value.i, pair->code);
          break;
        default:
          LOG_ERROR ("Unknown DXF code %d for %s", pair->code, "TABLESTYLE");
        }
      dxf_free_pair (pair);
      pair = dxf_read_pair (dat);
    }
  return pair;
}