static int php_hash_xxh32_unserialize(
		php_hashcontext_object *hash, zend_long magic, const zval *zv)
{
	PHP_XXH32_CTX *ctx = (PHP_XXH32_CTX *) hash->context;
	int r = error;
	if (magic == PHP_HASH_SERIALIZE_MAGIC_SPEC
		&& (r = php_hash_unserialize_spec(hash, zv, PHP_XXH32_SPEC)) == SUCCESS
		&& ctx->s.memsize < 16) {
		return SUCCESS;
	} else {
		return r != SUCCESS ? r : -2000;
	}
}