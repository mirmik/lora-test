#include <hal/board.h>
#include <hal/irq.h>

#include <systime/systime.h>

#include <genos/sched.h>
#include <genos/ktimer.h>

#include <drivers/serial/uartring.h>

#include <genos/api.h>
#include <genos/schedee/autom.h>

genos::stm32_usart usart6(USART6, USART6_IRQn);
genos::stm32_usart usart2(USART2, USART2_IRQn);

char rxbuf[128];
char txbuf[512];
genos::uartring serial6("ser6", &usart6, {txbuf, 512}, {rxbuf, 128});

char rxbuf2[128];
char txbuf2[512];
genos::uartring serial2("ser2", &usart2, {txbuf2, 512}, {rxbuf2, 128});

void* blink(void*, int * state)
{
	board::sysled.toggle();
	msleep(200);
	return NULL;
}
genos::autom_schedee blink_sch(blink, nullptr);

void request(genos::uartring* ring, const char* str, char* ans)
{
	ring->print(str);
	ring->print("\r\n");

	while (1)
	{
		if (ring->avail())
		{
			char c;
			ring->read(&c, 1, 0);
			if (c == '\n')
				break;
			else
				*ans++ = c;
		}
	}

	*(--ans) = 0;
}

void readcycle(genos::uartring* ring, char* buf)
{
	char * star = buf;

	while (1)
	{
		while (1)
		{
			if (ring->avail())
			{
				char c;
				ring->read(&c, 1, 0);
				if (c == '\n')
					break;
				else
					*buf++ = c;
			}
		}

		*(--buf) = 0;
		buf = star;
		serial2.println(buf, IO_NOBLOCK);
		board::sysled.toggle();
	}
}

int main()
{
	char buf[512];

	board_init(STM32_FREQMODE_HSI);
	schedee_manager_init();

	dprln("onstart");

	usart6.init_gpio({GPIOA, (1 << 11)}, {GPIOA, (1 << 12)}, 8);
	dprln("onstart1");
	usart6.setup(115200, 'n', 8, 1);
	usart2.setup(115200, 'n', 8, 1);

	dprln("onstart2");
	serial6.begin();
	serial2.begin();

	dprln("onstart3");
	blink_sch.start();
	dprln("onstart4");
	board::sysled.set(1);

	dprln("onstart5");
	irqs_enable();

	delay(1500);
	serial6.print("\r\n", 0);
	delay(500);


	serial6.print("\r\n", 0);
	//request(&serial6, "at+set_config=device:restart", buf);
	delay(10000);

//	request(&serial6, "at+run", buf);
//	serial2.println(buf, 0);
	dprln("startsend");

	serial6.read(buf, 512, IO_NOBLOCK);

	request(&serial6, "at+version", buf);
	serial2.println(buf, IO_NOBLOCK);
	delay(20);

	request(&serial6, "at+set_config=lora:region:EU868", buf);
	serial2.println(buf, IO_NOBLOCK);
	delay(20);

	request(&serial6, "at+set_config=lora:class:2", buf);
	serial2.println(buf, IO_NOBLOCK);
	delay(20);

	request(&serial6, "at+set_config=lora:work_mode:1", buf);
	serial2.println(buf, IO_NOBLOCK);
	delay(20);

	request(&serial6, "at+set_config=lorap2p:869525000:7:1:1:6:20", buf);
	serial2.println(buf, IO_NOBLOCK);
	delay(20);

	board::sysled.set(0);

#if 1

	serial2.println("send", IO_NOBLOCK);
	request(&serial6, "at+send=lorap2p:1234567890", buf);
	serial2.println(buf, IO_NOBLOCK);
	delay(200);

	serial6.read(buf, 512, IO_NOBLOCK);

	while (1)
	{
		serial2.println("send", IO_NOBLOCK);
		request(&serial6, "at+send=lorap2p:1234567890", buf);
		serial2.println(buf, IO_NOBLOCK);

		board::sysled.toggle();
		delay(70);
	}
#else
	readcycle(&serial6, buf);
#endif


	while (1)
		__schedule__();
}

void __schedule__()
{
	ktimer_manager_step();
	schedee_manager_step();
}