		gc_stack_free(&stack);

		if (!GC_G(num_roots)) {
			/* nothing to free */
			GC_TRACE("Nothing to free");
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
					cyclescollected -= gc_remove_nested_data_from_buffer(p, current);
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
