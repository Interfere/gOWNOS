// 
// isr.c -- Высокоуровневый обработчик прерываний
//

#include "common.h"
#include "isr.h"
#include "monitor.h"

static isr_t interrupt_handlers[256];

// Данная функция вызывается из нашего обработчика из файла interrupt.h
void isr_handler(registers_t regs)
{
	monitor_write("recieved interrupt: ");
	monitor_write_dec(regs.int_no);
	monitor_put('\n');

	if(interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}
}

void irq_handler(registers_t regs)
{
	// Посылаем контроллеру прерываний сигнал EOI (end of interrupt)
	// если прерываний пришло от второго контроллера (slave)
	if (regs.int_no >= 40)
	{
		// Посылаем сигнал reset второму контроллеру (slave)
		outb(0xA0,0x20);
	}
	// первому контроллеру (master) посылаем сигнал reset в любом случае
	outb(0x20,0x20);

	if(interrupt_handlers[regs.int_no] != 0)
	{
		isr_t handler = interrupt_handlers[regs.int_no];
		handler(regs);
	}
}

void register_interrupt_handler(u8int n, isr_t handler)
{
	interrupt_handlers[n] = handler;
}

