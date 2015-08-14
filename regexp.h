/* requires windows.h */

/*
	regexp function
	ported over from github.com/suosuopuo/regexp

	Usage:
		RE *re;
		re = compile("[abc]+");
		if(!re) {
			goto end;
		}
		if(match(re, "aaa")) {
			match
		} else {
			no match
		}
		regexp_free(re);
*/

typedef enum {
	RE_CHAR,
	RE_DOT,
	RE_BEGIN,
	RE_END,
	RE_STAR,
	RE_PLUS,
	RE_QUES,
	RE_LIST,
	RE_TYPENUM
} RE_TYPE;

typedef struct RE {
	RE_TYPE type;
	int ch;
	char *ccl;
	int nccl;
	struct RE *next;
} RE;

static int match_longest = 0;
static char *match_first = NULL;
static char *match_last  = NULL;

static void *getmem(size_t size)
{
	void *tmp;
	tmp = HeapAlloc(GetProcessHeap(), 0, size);
	if(!tmp) {
		ExitProcess(0);
	}
	return tmp;
}

static size_t create_list(char *str, int start, int end)
{
	size_t cnt = end - start + 1;
	for(; start <= end ; start++) {
		*str++ = start;
	}
	return (cnt > 0)?cnt:0;
}

static int in_list(char ch, RE *regexp)
{
	char *str = regexp->ccl;
	if(regexp->type != RE_LIST) {
		return 0;
	}
	for(; *str && ch != *str; str++)
		;
	return (*str != '\0') ^ regexp->nccl;
}

static void regexp_free(RE *regexp)
{
	RE *tmp;
	for(; regexp; regexp = tmp) {
		tmp = regexp->next;
		HeapFree(GetProcessHeap(), 0, regexp);
	}
}

static RE *compile(char *regexp)
{
	RE head, *tail, *tmp;
	char *pstr;
	int err_flag = 0;

	for(tail = &head; *regexp != '\0' && err_flag == 0; regexp++) {
		tmp = getmem(sizeof(RE));
		switch(*regexp) {
		case '\\':
			regexp++;
			if(*regexp == 'd') {
				tmp->type = RE_LIST;
				tmp->nccl = 0;
				tmp->ccl = getmem(11);
				create_list(tmp->ccl, '0','9');
				tmp->ccl[11] = '\0';
			} else if(*regexp == 'D') {
				tmp->type = RE_LIST;
				tmp->nccl = 1;
				tmp->ccl = getmem(11);
				create_list(tmp->ccl, '0','9');
				tmp->ccl[11] = '\0';
			} else {
				tmp->type = RE_CHAR;
				tmp->ch = *(regexp + 1);
			}
			break;
		case '.':
			tmp->type = RE_DOT;
			break;
		case '^':
			tmp->type = RE_BEGIN;
			tmp->ch = '^';
			break;
		case '$':
			tmp->type = RE_END;
			tmp->ch = '$';
			break;
		case '*':
			tmp->type = RE_STAR;
			break;
		case '+':
			tmp->type = RE_PLUS;
			break;
		case '?':
			tmp->type = RE_QUES;
			break;
		case '[':
			pstr = tmp->ccl = getmem(256);
			tmp->nccl = 0;
			if(*++regexp == '^') {
				tmp->nccl = 1;
				regexp++;
			}
			while(*regexp != '\0' && *regexp != ']') {
				if(*regexp != '-') {
					*pstr++ = *regexp++;
					continue;
				}
				if(pstr == tmp->ccl || *(regexp + 1) == ']') {
					err_flag = 1;
					break;
				}
				pstr += create_list(pstr, *(regexp - 1) + 1, *(regexp + 1));
				regexp += 2;
			}
			*pstr = '\0';
			if(*regexp == '\0') {
				err_flag = 1;
			}
			tmp->type = RE_LIST;
			break;
		default:
			tmp->type = RE_CHAR;
			tmp->ch = *regexp;
		}

		tail->next = tmp;
		tail = tmp;
	}

	tail->next = NULL;
	if(err_flag) {
		regexp_free(head.next);
		return NULL;
	}
	return head.next;
}

#define MATCH_ONE(reg, text) \
   	(reg->type == RE_DOT || in_list(*text, reg) || *text == reg->ch)
#define MATCH_ONE_P(reg, text) \
   	(in_list(*text++, reg) || *(text - 1) == reg->ch || reg->type == RE_DOT)

static int matchhere(RE *regexp, char *text);

static int matchstar(RE *cur, RE *regexp, char *text)
{
	do {
		if(matchhere(regexp, text)) {
			return 1;
		}
	} while(*text != '\0' && MATCH_ONE_P(cur, text));
	return 0;
}

static int matchstar_l(RE *cur, RE *regexp, char *text)
{
	char *t;
	for(t = text; *t != '\0' && MATCH_ONE(cur, t); t++);
	do {
		if(matchhere(regexp, t)) {
			return 1;
		}
	} while(t-- > text);
	return 0;
}

static int matchplus(RE *cur, RE *regexp, char *text)
{
	while(*text != '\0' && MATCH_ONE_P(cur, text)) {
		if(matchhere(regexp, text)) {
			return 1;
		}
	}
	return 0;
}

static int matchplus_l(RE *cur, RE *regexp, char *text)
{
	char *t;
	for(t = text; *t != '\0' && MATCH_ONE(cur, t); t++)
		;
	for(; t > text; t--) {
		if(matchhere(regexp, t)) {
			return 1;
		}
	}
	return 0;
}

static int matchques(RE *cur, RE *regexp, char *text)
{
	int cnt = 1;
	char *t = text;
	if(*t != '\0' && cnt-- && MATCH_ONE(cur, t)) {
		t++;
	}
	do {
		if(matchhere(regexp, t)) {
			return 1;
		}
	} while(t-- > text);
	return 0;
}

static int (*matchfun[RE_TYPENUM][2])(RE *, RE *, char *) = {
	0, 0, 0, 0, 0, 0, 0, 0,
	matchstar, matchstar_l,
	matchplus, matchplus_l,
	matchques, matchques,
};

static int matchhere(RE *regexp, char *text)
{
	match_last = text;
	if(regexp == NULL) {
		return 1;
	}
	if(regexp->type == RE_END && regexp->next == NULL) {
		return *text == '\0';
	}
	if(regexp->next && matchfun[regexp->next->type][match_longest]) {
		return matchfun[regexp->next->type][match_longest](regexp, regexp->next->next, text);
	}

	if(*text != '\0' && MATCH_ONE(regexp, text)) {
		return matchhere(regexp->next, text + 1);
	}
	return 0;
}

static int match(RE *regexp, char *text)
{
	int ret;
	if(regexp->type == RE_BEGIN) {
		ret = matchhere(regexp->next, text);
		match_first = text;
		goto out;
	}
	do {
		if(ret = matchhere(regexp, text)) {
			match_first = text;
			goto out;
		}
	} while(*text++ != '\0');
out:
	return ret;
}
