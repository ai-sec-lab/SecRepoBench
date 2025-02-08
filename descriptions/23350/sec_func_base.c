ZEND_API int zend_gc_collect_cycles(void)
{
	int count = 0;

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
		count = gc_collect_roots(&gc_flags, &stack);

		if (!GC_G(num_roots)) {
			/* nothing to free */
			GC_TRACE("Nothing to free");
			gc_stack_free(&stack);
			zend_get_gc_buffer_release();
			GC_G(gc_active) = 0;
			return 0;
		}

		end = GC_G(first_unused);

		if (gc_flags & GC_HAS_DESTRUCTORS) {
			GC_TRACE("Calling destructors");

			/* During a destructor call, new externally visible references to nested data may
			 * be introduced. These references can be introduced in a way that does not
			 * modify any refcounts, so we have no real way to detect this situation
			 * short of rerunning full GC tracing. What we do instead is to only run
			 * destructors at this point, and leave the actual freeing of the objects
			 * until the next GC run. */

			/* Mark all roots for which a dtor will be invoked as DTOR_GARBAGE. Additionally
			 * color them purple. This serves a double purpose: First, they should be
			 * considered new potential roots for the next GC run. Second, it will prevent
			 * their removal from the root buffer by nested data removal. */
			idx = GC_FIRST_ROOT;
			current = GC_IDX2PTR(GC_FIRST_ROOT);
			while (idx != end) {
				if (GC_IS_GARBAGE(current->ref)) {
					p = GC_GET_PTR(current->ref);
					if (GC_TYPE(p) == IS_OBJECT && !(OBJ_FLAGS(p) & IS_OBJ_DESTRUCTOR_CALLED)) {
						zend_object *obj = (zend_object *) p;
						if (obj->handlers->dtor_obj != zend_objects_destroy_object
							|| obj->ce->destructor) {
							current->ref = GC_MAKE_DTOR_GARBAGE(obj);
							GC_REF_SET_COLOR(obj, GC_PURPLE);
						} else {
							GC_ADD_FLAGS(obj, IS_OBJ_DESTRUCTOR_CALLED);
						}
					}
				}
				current++;
				idx++;
			}

			/* Remove nested data for objects on which a destructor will be called.
			 * This will not remove the objects themselves, as they have been colored
			 * purple. */
			idx = GC_FIRST_ROOT;
			current = GC_IDX2PTR(GC_FIRST_ROOT);
			while (idx != end) {
				if (GC_IS_DTOR_GARBAGE(current->ref)) {
					p = GC_GET_PTR(current->ref);
					count -= gc_remove_nested_data_from_buffer(p, current, &stack);
				}
				current++;
				idx++;
			}

			/* Actually call destructors.
			 *
			 * The root buffer might be reallocated during destructors calls,
			 * make sure to reload pointers as necessary. */
			idx = GC_FIRST_ROOT;
			while (idx != end) {
				current = GC_IDX2PTR(idx);
				if (GC_IS_DTOR_GARBAGE(current->ref)) {
					p = GC_GET_PTR(current->ref);
					/* Mark this is as a normal root for the next GC run,
					 * it's no longer garbage for this run. */
					current->ref = p;
					/* Double check that the destructor hasn't been called yet. It could have
					 * already been invoked indirectly by some other destructor. */
					if (!(OBJ_FLAGS(p) & IS_OBJ_DESTRUCTOR_CALLED)) {
						zend_object *obj = (zend_object*)p;
						GC_TRACE_REF(obj, "calling destructor");
						GC_ADD_FLAGS(obj, IS_OBJ_DESTRUCTOR_CALLED);
						GC_ADDREF(obj);
						obj->handlers->dtor_obj(obj);
						GC_DELREF(obj);
					}
				}
				idx++;
			}

			if (GC_G(gc_protected)) {
				/* something went wrong */
				zend_get_gc_buffer_release();
				return 0;
			}
		}

		gc_stack_free(&stack);

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
		GC_G(collected) += count;
		GC_G(gc_active) = 0;
	}

	gc_compact();
	zend_get_gc_buffer_release();
	return count;
}