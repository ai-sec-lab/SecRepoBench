if (old_index <= 257) new_index = old_index;
    else if (old_new_index_map.has (old_index, &new_index2))
    {
      new_index = *new_index2;
    } else {
      hb_bytes_t s = _post.find_glyph_name (old_gid);
      new_index = glyph_name_to_new_index.get (s);
      if (new_index == (unsigned)-1)
      {
        int standard_glyph_index = -1;
        for (unsigned i = 0; i < format1_names_length; i++)
        {
          if (s == format1_names (i))
          {
            standard_glyph_index = i;
            break;
          }
        }

        if (standard_glyph_index == -1)
        {
          new_index = 258 + i;
          i++;
        }
        else
        { new_index = standard_glyph_index; }
        glyph_name_to_new_index.set (s, new_index);
      }
      old_new_index_map.set (old_index, new_index);
    }