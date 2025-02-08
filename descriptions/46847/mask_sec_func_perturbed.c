static Dxf_Pair *
add_TABLEGEOMETRY_Cell (Dwg_Object *restrict obj, Bit_Chain *restrict dat,
                        Dxf_Pair *restrict dxfpair)
{
  Dwg_Data *dwg = obj->parent;
  Dwg_Object_TABLEGEOMETRY *o = obj->tio.object->tio.TABLEGEOMETRY;
  BITCODE_H hdl;
  BITCODE_BL num_cells = o->num_cells;
  int i = -1, j = -1;
	
  // <MASK>
}