#include "timer.h"
#include "isr.h"
#include "monitor.h"

static u32int tick = 0;

static void timer_callback(registers_t regs)
{
	tick++;
	monitor_write("Tick: ");
	monitor_write_dec(tick);
	monitor_write("\n");
}

void init_timer(u32int freq)
{
	// Для начала регистрируем наш callback
	register_interrupt_handler(IRQ0,&timer_callback);

	// Значение, сообщаемое в PIT
	u32int divisor = 1193180 / freq;

	// Послать команду
	outb(0x43,0x36);

	// Значение посылается в два этапа
	u8int l = (u8int)(divisor & 0xFF);
	u8int h = (u8int)((divisor>>8) & 0xFF);

	// Посылаем на порт данных
	outb(0x40,l);
	outb(0x40,h);
}
