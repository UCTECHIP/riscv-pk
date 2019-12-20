// See LICENSE for license details.

#include <string.h>
#include "uart.h"
#include "fdt.h"


volatile uint32_t* uart;

#define readb(a)			(*(unsigned char *)(a))
#define writeb(v, a)		(*(unsigned char *)(a) = (v))


struct uart_wh {
	char is;
	char ie;
	char con;
	char data;
	char bprl;
	char bprh;
};

void uart_delay(int nms)
{
	volatile int i;

	while(nms--)
	{
		for(i = 0; i < 12; i++)
			;
	}
}


void uart_putchar(uint8_t ch)
{
        struct uart_wh *regs = (struct uart_wh *)uart;
	writeb((readb(&regs->con)|(UART_CON_TRS)),&regs->con);

	writeb(ch,&regs->data);
	uart_delay(1000);

	//while(!(readb(&regs->is) & UART_IS_TXEND))
	//	continue;

	//writeb((readb(&regs->is) & (~UART_IS_TXEND)),&regs->is);
	//writeb(((readb(&regs->bprh) & 0x0F) |0x50),&regs->bprh);//set UART_ECR with 3
	//writel((0x30 |br_h),&regs->bprh);//equal the last line code,but the curcuit have a little bug when read the device, so use this line code

	//while(!(readb(&regs->is) & UART_IS_ECNT0));	//the etu didn't arrive default value(3)

	//writeb((readb(&regs->is) & ~UART_IS_ECNT0),&regs->is);

  
}

int uart_getchar()
{
    int ch;
	struct uart_wh *regs = (struct uart_wh *)uart;
	
	writeb((readb(&regs->con) & (~UART_CON_TRS)),&regs->con); //select receive mode

	if(readb(&regs->is) & UART_IS_TRE)
		writeb((readb(&regs->is) & (~UART_IS_TRE)),&regs->is);
	if(readb(&regs->is) & UART_IS_FIFO_OV)
		writeb((readb(&regs->con) | UART_CON_FLUSH),&regs->con);  //uart_fflush();
/*
	while(!(readb(&regs->is) & UART_IS_FIFO_NE))
		continue;

	writeb((readb(&regs->ie) | UART_IE_FIFO_NE),&regs->ie);

	return readb(&regs->data);
*/
	if (!(readb(&regs->is) & UART_IS_FIFO_NE))
		return -1;
	writeb((readb(&regs->ie) | UART_IE_FIFO_NE),&regs->ie);
	ch=readb(&regs->data);
	return (!ch) ? -1 : ch;
 }

struct uart_scan
{
  int compat;
  uint64_t reg;
};

static void uart_open(const struct fdt_scan_node *node, void *extra)
{
  struct uart_scan *scan = (struct uart_scan *)extra;
  memset(scan, 0, sizeof(*scan));
}

static void uart_prop(const struct fdt_scan_prop *prop, void *extra)
{
  struct uart_scan *scan = (struct uart_scan *)extra;
  if (!strcmp(prop->name, "compatible") && !strcmp((const char*)prop->value, "wh,uart0")) {
    scan->compat = 1;
  } else if (!strcmp(prop->name, "reg")) {
    fdt_get_address(prop->node->parent, prop->value, &scan->reg);
  }
}

static void uart_done(const struct fdt_scan_node *node, void *extra)
{
  struct uart_scan *scan = (struct uart_scan *)extra;
  int br;
  char br_l,br_h;

  if (!scan->compat || !scan->reg ) return;

  // Enable Rx/Tx channels
  uart = (void*)(uintptr_t)scan->reg;
  struct uart_wh *regs = (struct uart_wh *)uart;
  br = CORE_FREQ / baud_rate;
  br_l = (char)(br % 256);
  br_h = (char)(br / 256);
  writeb(br_l,&regs->bprl);
  writeb(((readb(&regs->bprh)) & 0XF0) | br_h,&regs->bprh);
  
  //if(uart != (void*)(uintptr_t)0x10000010)
	//  uart = (void*)(uintptr_t)0x10000010;
  //uart = (void*)(uintptr_t)0x10000010;
  //uart[UART_REG_TXCTRL] = UART_TXEN;
  //uart[UART_REG_RXCTRL] = UART_RXEN;
}

void query_uart(uintptr_t fdt)
{
  struct fdt_cb cb;
  struct uart_scan scan;

  memset(&cb, 0, sizeof(cb));
  cb.open = uart_open;
  cb.prop = uart_prop;
  cb.done = uart_done;
  cb.extra = &scan;

  fdt_scan(fdt, &cb);
}
