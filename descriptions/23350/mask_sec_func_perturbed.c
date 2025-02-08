ZEND_API int zend_gc_collect_cycles(void)
{
	int cyclescollected = 0;

	if (GC_G(num_roots)) {
		gc_root_buffer *current, *last;
		zend_refcounted *p;
		uint32_t gc_flags = 0;
		uint32_t idx, end;
		gc_stack stack;

		stack.prev = NULL;
		stack.next = NULL;

		if (GC_G(gc_active)) {
			return 0;
		}

		GC_TRACE("Collecting cycles");
		GC_G(gc_runs)++;
		GC_G(gc_active) = 1;

		GC_TRACE("Marking roots");
		gc_mark_roots(&stack);
		GC_TRACE("Scanning roots");
		gc_scan_roots(&stack);

		GC_TRACE("Collecting roots");
		cyclescollected = gc_collect_roots(&gc_flags, &stack);

		// <MASK>

		/* Destroy zvals. The root buffer may be reallocated. */
		GC_TRACE("Destroying zvals");
		idx = GC_FIRST_ROOT;
		while (idx != end) {
			current = GC_IDX2PTR(idx);
			if (GC_IS_GARBAGE(current->ref)) {
				p = GC_GET_PTR(current->ref);
				GC_TRACE_REF(p, "destroying");
				if (GC_TYPE(p) == IS_OBJECT) {
					zend_object *obj = (zend_object*)p;

					EG(objects_store).object_buckets[obj->handle] = SET_OBJ_INVALID(obj);
					GC_TYPE_INFO(obj) = IS_NULL |
						(GC_TYPE_INFO(obj) & ~GC_TYPE_MASK);
					/* Modify current before calling free_obj (bug #78811: free_obj() can cause the root buffer (with current) to be reallocated.) */
					current->ref = GC_MAKE_GARBAGE(((char*)obj) - obj->handlers->offset);
					if (!(OBJ_FLAGS(obj) & IS_OBJ_FREE_CALLED)) {
						GC_ADD_FLAGS(obj, IS_OBJ_FREE_CALLED);
						GC_ADDREF(obj);
						obj->handlers->free_obj(obj);
						GC_DELREF(obj);
					}

					ZEND_OBJECTS_STORE_ADD_TO_FREE_LIST(obj->handle);
				} else if (GC_TYPE(p) == IS_ARRAY) {
					zend_array *arr = (zend_array*)p;

					GC_TYPE_INFO(arr) = IS_NULL |
						(GC_TYPE_INFO(arr) & ~GC_TYPE_MASK);

					/* GC may destroy arrays with rc>1. This is valid and safe. */
					HT_ALLOW_COW_VIOLATION(arr);

					zend_hash_destroy(arr);
				}
			}
			idx++;
		}

		/* Free objects */
		current = GC_IDX2PTR(GC_FIRST_ROOT);
		last = GC_IDX2PTR(end);
		while (current != last) {
			if (GC_IS_GARBAGE(current->ref)) {
				p = GC_GET_PTR(current->ref);
				GC_LINK_UNUSED(current);
				GC_G(num_roots)--;
				efree(p);
			}
			current++;
		}

		GC_TRACE("Collection finished");
		GC_G(collected) += cyclescollected;
		GC_G(gc_active) = 0;
	}

	gc_compact();
	zend_get_gc_buffer_release();
	return cyclescollected;
}