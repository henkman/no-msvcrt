// requires: windows.h

/*
	Parses the command line passed to the program, matches keys
	and puts the parsed key value pairs into $args.

	sample usage:
		Arg prog, regex, base, ignorecase;
		Arg *args[] = {&prog, &regex, &base, &ignorecase};
		args_make(&prog, ArgProg, NULL);
		args_make(&search, ArgValue, "-s");
		args_make(&base, ArgValue, "-b");
		args_make(&ignorecase, ArgBool, "-ic");
		args_get(args, ARR_LEN(args), GetCommandLine());
		if(prog.value) {
			prog.value has program path and name
			prog.len the length of prog.value
		}
		if(regex.value) {
			regex.value has the string
			regex.len has the length of regex.value
		}
		if(ignorecase.isset) {
			switch for ignorecase was set
		}
		[..]

	sample parameters to the program and their parsed key value pairs:
		$ prog -s \.jpg$ -ic
		-> prog.value = prog
		-> prog.len = 4
		-> regex.value = \.jpg$
		-> regex.len = 6
		-> base.value = NULL
		-> base.len = 0
		-> ignorecase.isset = 1

		$ prog -b "C:\projects\hello world"
		-> prog.value = prog
		-> prog.len = 4
		-> regex.value = NULL
		-> regex.len = 0
		-> base.value = C:\projects\hello world
		-> base.len = 23
		-> ignorecase.isset = 0
*/

typedef enum {
	ArgProg,
	ArgBool,
	ArgValue
} ArgType;

typedef struct {
	ArgType type;
	char *key;
	union {
		struct {
			char *value;
			int len;
		};
		int isset;
	};
} Arg;

static void args_make(Arg *arg, ArgType type, char *key)
{
	arg->type = type;
	arg->key = key;
	if(type == ArgBool) {
		arg->isset = 0;
	} else if(type == ArgValue || type == ArgProg) {
		arg->value = NULL;
		arg->len = 0;
	}
}

static void args_get(Arg **args, int count, const char *cmdline)
{
	static char cmd[8*1024];
	int i, o, n, ignorespace;
	Arg *key;

	lstrcpy(cmd, cmdline);

	/* parse program name */
	o = 0;
	while(cmd[o] && cmd[o] != ' ') {
		o++;
	}
	cmd[o] = 0;
	for(i=0; i<count; i++) {
		if(args[i]->type == ArgProg) {
			args[i]->value = &cmd[0];
			args[i]->len = o;
			break;
		}
	}
	o++;
	if(!cmd[o]) {
		return;
	}
	while(cmd[o] == ' ' || cmd[o] == '\t') {
		o++;
	}

	/* begin key value pairs */
	key = NULL;
	n = o;
	ignorespace = 0;
	while(cmd[o]) {
		if(cmd[o] == '"') {
			ignorespace = !ignorespace;
			o++;
		} else if(cmd[o] == ' ' && !ignorespace) {
			cmd[o] = 0;
			if(key) {
				key->value = &cmd[n];
				key->len = o-n;
				if(key->value[0] == '"') {
					key->value++;
					key->len--;
				}
				if(key->value[key->len-1] == '"') {
					key->value[--key->len] = 0;
				}
				key = NULL;
			} else {
				for(i=0; i<count; i++) {
					if(lstrcmp(args[i]->key, &cmd[n]) == 0) {
						if(args[i]->type == ArgBool) {
							args[i]->isset = 1;
						} else {
							key = args[i];
						}
						break;
					}
				}
			}
			o++;
			if(!cmd[o]) {
				break;
			}
			while(cmd[o] == ' ' || cmd[o] == '\t') {
				o++;
			}
			n = o;
		} else {
			o++;
		}
	}

	/* the last value, if any */
	if(key) {
		key->value = &cmd[n];
		key->len = o-n;
		if(key->value[0] == '"') {
			key->value++;
			key->len--;
		}
		if(key->value[key->len-1] == '"') {
			key->value[--key->len] = 0;
		}
	} else {
		for(i=0; i<count; i++) {
			if(lstrcmp(args[i]->key, &cmd[n]) == 0) {
				if(args[i]->type == ArgBool) {
					args[i]->isset = 1;
				}
				break;
			}
		}
	}
}
