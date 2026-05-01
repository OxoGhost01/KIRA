#include "defs.h"

/* panic() : unrecoverable kernel error handler.
 *
 * When something goes horribly wrong and there is no path forward,
 * call this. It prints the message over UART and spins forever.
 * On real hardware you would want to trigger a reset, but for us
 * a spinning CPU is fine : at least QEMU stays alive so we can
 * attach GDB and see what exploded.
 */
void panic(const char *msg) {
    uart_puts(msg);
    while (1);
}
