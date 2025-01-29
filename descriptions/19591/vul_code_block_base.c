name = pe_get_dotnet_string(
          pe, string_offset, DOTNET_STRING_INDEX(module_table->Name));

      if (name != NULL)
        set_string(name, pe->object, "module_name");

      table_offset += (2 + index_sizes.string + (index_sizes.guid * 3)) *
                      num_rows;

      break;