/* su_led.c -- LED control. SPDX-License-Identifier: MIT
 *
 * Two flows, chosen at run time like the vendor libraries do per SoC:
 *  - /proc/board/power/led<N> (T10/T20/T21/T23/T31/T41 vendor flow) when that node exists;
 *  - sysfs GPIO (T40 vendor libsysutils 1.3.x flow): export N, active_low 0, direction out,
 *    value; cmd 2 writes 0 and unexports.  Used when the proc node is absent. */

#include "sysutils/su_misc.h"
#include "su_internal.h"

#define LED_FMT "/proc/board/power/led%d"

#ifndef SU_LED_GPIO_DIR
#define SU_LED_GPIO_DIR "/sys/class/gpio/"
#endif

/* the vendor writes a fixed buffer and ignores the write result; only open() decides */
static int gpio_write(const char *path, const char *value)
{
	char buf[64];
	int fd = open_cloexec(path, O_WRONLY);
	if (fd < 0)
		return -1;
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%s", value);
	(void)!write(fd, buf, sizeof(buf));
	close(fd);
	return 0;
}

static int led_gpio(int ledNum, SULedCmd cmd)
{
	char path[256], val[16];

	snprintf(val, sizeof(val), "%d", ledNum);
	if (gpio_write(SU_LED_GPIO_DIR "export", val) < 0)
		return -1;
	snprintf(path, sizeof(path), SU_LED_GPIO_DIR "gpio%d/active_low", ledNum);
	if (gpio_write(path, "0") < 0)
		return -1;
	snprintf(path, sizeof(path), SU_LED_GPIO_DIR "gpio%d/direction", ledNum);
	if (gpio_write(path, "out") < 0)
		return -1;
	snprintf(path, sizeof(path), SU_LED_GPIO_DIR "gpio%d/value", ledNum);
	snprintf(val, sizeof(val), "%d", (int)cmd > 1 ? ((int)cmd != 2) : (int)cmd);
	if (gpio_write(path, val) < 0)
		return -1;
	if ((int)cmd != 2)
		return 0;
	snprintf(val, sizeof(val), "%d", ledNum);
	return gpio_write(SU_LED_GPIO_DIR "unexport", val);
}

EXPORT int SU_LED_Command(int ledNum, SULedCmd cmd)
{
	char path[64];
	snprintf(path, sizeof(path), LED_FMT, ledNum);
	if (access(path, F_OK) != 0 && errno == ENOENT)
		return led_gpio(ledNum, cmd);
	return write_file(path, (cmd == LED_ON) ? "1" : "0", 1);
}
