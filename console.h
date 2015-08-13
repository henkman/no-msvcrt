// requires windows.h

typedef struct {
	HANDLE stdout;
} Console;

static void console_init(Console *c)
{
	AllocConsole();
	c->stdout = GetStdHandle(STD_OUTPUT_HANDLE);
}

static void console_deinit(Console *c)
{
	FreeConsole();
}

static unsigned long console_write(Console *c, void *msg, size_t len)
{
	unsigned long n;
	WriteConsoleA(c->stdout, msg, len, &n, NULL);
	return n;
}

static unsigned long console_print(Console *c, void *msg)
{
	return console_write(c, msg, lstrlen(msg));
}

static unsigned long console_put(Console *c, char b)
{
	char msg[1] = {b};
	return console_write(c, msg, sizeof(msg));
}

static unsigned long console_println(Console *c, void *msg)
{
	unsigned long n;
	n = console_write(c, msg, lstrlen(msg));
	return n + console_put(c, '\n');
}
