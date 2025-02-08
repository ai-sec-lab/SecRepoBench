int
coolkey_compare_id(const void * a, const void *objectb)
{
	if (a == NULL || objectb == NULL)
		return 1;
	return ((sc_cardctl_coolkey_object_t *)a)->id
	    != ((sc_cardctl_coolkey_object_t *)objectb)->id;
}