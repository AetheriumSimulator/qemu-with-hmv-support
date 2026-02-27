/*
 * QEMU HMV support - HarmonyOS Fixes
 * AI Generated. Testing was not performed due to SELinux restrictions on HarmonyOS.
 */

#include "qemu/osdep.h"
#include <setjmp.h>

/* Global jump buffer for fatal exit recovery */
jmp_buf qemu_exit_sigjmp_buf;
bool qemu_exit_sigjmp_buf_valid = false;

/* Redirect exit calls to longjmp */
void exit(int status)
{
    if (qemu_exit_sigjmp_buf_valid) {
        longjmp(qemu_exit_sigjmp_buf, status ? status : -1);
    }
    /* Fallback if jump buffer isn't set? 
     * In a library context, we might just have to spin or log. 
     */
    while(1); 
}

void _exit(int status)
{
    exit(status);
}

void abort(void)
{
    exit(EXIT_FAILURE);
}
