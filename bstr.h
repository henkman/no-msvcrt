/*
	bstrcpy and bstrcat are like their C library counterparts strcpy and strcat
	but return a pointer to the null-terminator instead of one to the
	destination.
	This allows for faster chaining of concatenation because the search for
	the null-terminator is avoided.
	
	sample:
		char buf[];
		char *end;
		
		end = bstrcpy(buf, "hello");
		end = bstrcat(end, "world");
		bstrcat(end, ".\n");
*/
static char *bstrcpy(char *dst, const char *src)
{
	while(*src) {
		*dst++ = *src++;		
	}
	*dst = 0;
	return dst;
}

static char *bstrcat(char *dst, const char *src)
{
	while(*dst) {
		dst++;
	}
	while(*src) {
		*dst++ = *src++;		
	}
	*dst = 0;
	return dst;
}

/*
	Returns pointer to last occurence of c in s 
	or NULL if it was not found
*/
static char *findlast(const char *s, char c)
{
	char *p = (char *)s;
	while(*p) {
		p++;
	}
	p--;
	while(p != s) {
		if(*p == c) {
			return p;
		}
		p--;
	}
	return NULL;
}

/*
	Checks if s ends with c
*/
static int endswith(const char *s, char c)
{
	while(*s) {
		s++;
	}
	s--;
	return *s == c;
}

/*
	stou is like atoi but unsigned
*/
static unsigned stou(const char *s)
{
	char *p = s;
	unsigned r = 0; 
	while(*p) { 
		if(*p < '0' || *p > '9') { 
			return 0;
		} 
		r *= 10; 
		r += *p - '0'; 
		p++;
	} 
	return r; 
}
