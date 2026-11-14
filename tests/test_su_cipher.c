/*
 * test_su_cipher.c -- host test for SU_CIPHER_CreateHandle/DestroyHandle return codes.
 * Vendor libsysutils (T10/T20/T21/T23/T30/T32/T33 1.0.x, T40/T41 1.2-1.3): CreateHandle
 * without Init -> -12; DestroyHandle with no live handle -> -19; Create after Init -> fd.
 * /dev/aes is faked with /dev/null via --wrap=open. SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include "sysutils/su_cipher.h"

static int failures;
#define CHECK(c) do { if (!(c)) { printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #c); failures++; } } while (0)

int __real_open(const char *path, int flags, ...);
int __wrap_open(const char *path, int flags, ...)
{
	if (strcmp(path, "/dev/aes") == 0) return __real_open("/dev/null", flags);
	return -1;
}

int main(void)
{
	CHECK(SU_CIPHER_CreateHandle() == -12);
	CHECK(SU_CIPHER_DestroyHandle(-1) == -19);
	CHECK(SU_CIPHER_DestroyHandle(0) == -19);
	CHECK(SU_CIPHER_Init() == 0);
	CHECK(SU_CIPHER_DestroyHandle(3) == -19);   /* initialised but no live handle */
	int h = SU_CIPHER_CreateHandle();
	CHECK(h >= 0);
	CHECK(SU_CIPHER_DestroyHandle(h) == 0);
	CHECK(SU_CIPHER_DestroyHandle(h) == -19);   /* count back to 0 */
	CHECK(SU_CIPHER_Exit() == 0);
	if (!failures) printf("OK (0 failures)\n");
	return failures;
}
