#pragma once

/*void ioapic_enableall(void);
void ioapic_disableall(void);
int ioapic_irq_getvector(unsigned int numIrq);*/
void interrupt_handler_gen(unsigned int num, unsigned int * stack);
void systeminit(void);

