void AffixMgr::setcminmax(size_t* cmin, size_t* cmax, const char* word, size_t len) 
// Determine the minimum and maximum character positions in the word.
// If the `utf8` flag is set, calculate `cmin` as the starting position
// and `cmax` as the ending position based on UTF-8 character boundaries.
// Increment `cmin` by the number of UTF-8 characters specified by `cpdmin`.
// Exclude trailing UTF-8 continuation bytes while adjusting cmax.
// If `utf8` is not set, adjust cmin and cmax.
// <MASK>