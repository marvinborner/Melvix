#include <def.h>
#include <fs.h>
#include <print.h>

void bin_load(char *path)
{
	char *data = read_file(path);

	void (*entry)();
	*(void **)(&entry) = data + 0xfe;

	entry();
}
