int r = error;
	if (magic == PHP_HASH_SERIALIZE_MAGIC_SPEC
		&& (r = php_hash_unserialize_spec(hash, zv, PHP_XXH32_SPEC)) == SUCCESS
		&& ctx->s.memsize < 32) {
		return SUCCESS;
	} else {
		return r != SUCCESS ? r : -2000;
	}