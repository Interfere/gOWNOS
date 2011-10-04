// monitor.h -- Defines the interface for monitor

#ifndef MONITOR_H_
#define MONITOR_H_

#include "common.h"

// Write a single character out to the screen
extern void monitor_put(char c);

// Clear the screen to all back
extern void monitor_clear();

// Output the null-terminated ASCII string to the monitor
extern void monitor_write(char *c);

extern void monitor_write_hex(u32int n);

extern void monitor_write_dec(u32int n);

#endif

