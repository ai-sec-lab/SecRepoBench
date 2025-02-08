row_size = (2 + 2 + 2 + 2 + 4 +
            (index_sizes.blob * 2) +
            (index_sizes.string * 2));

        row_ptr = table_offset;

        for (i = 0; i < num_rows; i++)
        {
          if (!fits_in_pe(pe, row_ptr, row_size))
            break;

          assemblyref_table = (PASSEMBLYREF_TABLE) row_ptr;

          set_integer(assemblyref_table->MajorVersion,
              pe->object, "assembly_refs[%i].version.major", i);
          set_integer(assemblyref_table->MinorVersion,
              pe->object, "assembly_refs[%i].version.minor", i);
          set_integer(assemblyref_table->BuildNumber,
              pe->object, "assembly_refs[%i].version.build_number", i);
          set_integer(assemblyref_table->RevisionNumber,
              pe->object, "assembly_refs[%i].version.revision_number", i);

          blob_offset = pe->data + metadata_root + streams->blob->Offset;

          if (index_sizes.blob == 4)
            blob_offset += \
                assemblyref_table->PublicKeyOrToken.PublicKeyOrToken_Long;
          else
            blob_offset += \
                assemblyref_table->PublicKeyOrToken.PublicKeyOrToken_Short;

          blob_result = dotnet_parse_blob_entry(pe, blob_offset);
          blob_offset += blob_result.size;

          if (blob_result.size == 0 ||
              !fits_in_pe(pe, blob_offset, blob_result.length))
          {
            row_ptr += row_size;
            continue;
          }

          // Avoid empty strings.
          if (blob_result.length > 0)
          {
            set_sized_string((char*) blob_offset,
                blob_result.length, pe->object,
                "assembly_refs[%i].public_key_or_token", i);
          }

          // Can't use assemblyref_table here because the PublicKey comes before
          // Name and is a variable length field.

          if (index_sizes.string == 4)
            name = pe_get_dotnet_string(pe,
                string_offset,
                *(DWORD*) (row_ptr + 2 + 2 + 2 + 2 + 4 + index_sizes.blob));
          else
            name = pe_get_dotnet_string(pe,
                string_offset,
                *(WORD*) (row_ptr + 2 + 2 + 2 + 2 + 4 + index_sizes.blob));

          if (name != NULL)
            set_string(name, pe->object, "assembly_refs[%i].name", i);

          row_ptr += row_size;
        }