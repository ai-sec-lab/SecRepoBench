if (!subtable_record || !x_ppem || !y_ppem)
	return false;

      if (subtable_record->get_extents (extents, base))
	return true;

      unsigned int image_offset = 0, image_length = 0, image_format = 0;