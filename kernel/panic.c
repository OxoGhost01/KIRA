#import "defs.h"

void panic(char *msg) {
    uart_puts(msg);
    while(1);
}