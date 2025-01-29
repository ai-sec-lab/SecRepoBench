ligature_count++;
      if (!glyphset.has (gid)) continue;

      auto *matrix = out->serialize_append (c->serializer);
      if (unlikely (!matrix)) return_trace (false);

      matrix->serialize_subset (c,
				this->arrayZ[ligature_count - 1],
				this,
				class_count,
				klass_mapping);