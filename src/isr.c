// 
// isr.c -- Высокоуровневый обработчик прерываний
//

#include "common.h"
#include "isr.h"
#include "monitor.h"

// Данная функция вызывается из нашего обработчика из файла interrupt.h
void isr_handler(registers_t regs)
{
	monitor_write("recieved interrupt: ");
	monitor_write_dec(regs.int_no);
	monitor_put('\n');
}

