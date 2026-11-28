/* test_su_key.c -- host test: SU_Key_Enable/DisableEvent on a gpio-keys device without keys
 * (T33L module: "keys" reads "\n"). Vendor libsysutils T33 2.0.2.1: load_globle_keys 0x7e20
 * succeeds with 0 keys, update_enabled_keys 0x8364 returns 0 without sysfs I/O (0x83d4) ->
 * both calls answer 0 (device A/B 2026-11-28, #995). Links su_key.c with --wrap=open,read,access,write. */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "sysutils/su_misc.h"

static int failures, writes;
static const char *keys_content = "\n";
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

int __wrap_access(const char *path, int mode)
{
	(void)mode;
	return strcmp(path, "/sys/devices/platform/gpio-keys") == 0 ? 0 : -1;
}
int __wrap_open(const char *path, int flags, ...)
{
	(void)flags;
	if (!strcmp(path, "/sys/devices/platform/gpio-keys/keys")) return 200;
	if (!strcmp(path, "/sys/devices/platform/gpio-keys/disabled_keys")) return 201;
	return -1;
}
ssize_t __real_read(int fd, void *buf, size_t n);
ssize_t __wrap_read(int fd, void *buf, size_t n)
{
	if (fd != 200) return __real_read(fd, buf, n);
	size_t l = strlen(keys_content);
	if (l > n) l = n;
	memcpy(buf, keys_content, l);
	return (ssize_t)l;
}
ssize_t __real_write(int fd, const void *buf, size_t n);
ssize_t __wrap_write(int fd, const void *buf, size_t n)
{
	if (fd != 201) return __real_write(fd, buf, n);
	writes++;
	return (ssize_t)n;
}
int __real_close(int fd);
int __wrap_close(int fd) { return (fd == 200 || fd == 201) ? 0 : __real_close(fd); }

int main(void)
{
	CHECK(SU_Key_EnableEvent(9999) == 0);
	CHECK(SU_Key_DisableEvent(9999) == 0);
	CHECK(SU_Key_EnableEvent(116) == 0);
	CHECK(writes == 0);	/* vendor: no sysfs write without keys */
	printf("%s (%d failures)\n", failures ? "FAILED" : "OK", failures);
	return failures != 0;
}
