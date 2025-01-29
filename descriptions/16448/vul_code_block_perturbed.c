case BIT_MODULEREF:
        row_ptr = table_offset;

        // Can't use 'i' here because we only set the string if it is not
        // NULL. Instead use 'counter'.
        counter = 0;

        for (i = 0; i < num_rows; i++)
        {
          moduleref_table = (PMODULEREF_TABLE) row_ptr;

          name = pe_get_dotnet_string(pe,
              string_offset,
              DOTNET_STRING_INDEX(moduleref_table->Name));

          if (name != NULL)
          {
            set_string(name, pe->object, "modulerefs[%i]", counter);
            counter++;
          }

          row_ptr += index_sizes.string;
        }

        set_integer(counter, pe->object, "number_of_modulerefs");

        table_offset += (index_sizes.string) * num_rows;
        break;

      case BIT_TYPESPEC:
        table_offset += (index_sizes.blob) * num_rows;
        break;

      case BIT_IMPLMAP:
        row_count = max_rows(2,
            yr_le32toh(rows.field),
            yr_le32toh(rows.methoddef));

        if (row_count > (0xFFFF >> 0x01))
          index_size = 4;
        else
          index_size = 2;

        table_offset += (
            2 + index_size + index_sizes.string +
            index_sizes.moduleref) * num_rows;
        break;

      case BIT_FIELDRVA:
        row_size = 4 + index_sizes.field;
        row_ptr = table_offset;

        // Can't use 'i' here because we only set the field offset if it is
        // valid. Instead use 'counter'.
        counter = 0;

        for (i = 0; i < num_rows; i++)
        {
          fieldrva_table = (PFIELDRVA_TABLE) row_ptr;

          field_offset = pe_rva_to_offset(pe, fieldrva_table->RVA);
          if (field_offset >= 0)
          {
            set_integer(field_offset, pe->object, "field_offsets[%i]", counter);
            counter++;
          }

          row_ptr += row_size;
        }

        set_integer(counter, pe->object, "number_of_field_offsets");

        table_offset += row_size * num_rows;
        break;