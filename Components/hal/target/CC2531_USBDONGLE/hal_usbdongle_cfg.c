#include "hal_usbdongle_cfg.h"


void  HAL_BOARD_INIT(void)
{                         
  uint16 i;               
							
  SLEEPCMD &= ~OSC_PD;                       /* turn on 16MHz RC and 32MHz XOSC */               
  while (!(SLEEPSTA & XOSC_STB));            /* wait for 32MHz XOSC stable */                    
  asm("NOP");                                /* chip bug workaround */                            
  for (i=0; i<504; i++) asm("NOP");          /* Require 63us delay for all revs */                
  CLKCONCMD = (CLKCONCMD_32MHZ | OSC_32KHZ); /* Select 32MHz XOSC and the source for 32K clock */ 
  while (CLKCONSTA != (CLKCONCMD_32MHZ | OSC_32KHZ)); /* Wait for the change to be effective */   
  SLEEPCMD |= OSC_PD;                        /* turn off 16MHz RC */                              
                                                                 
  /* Turn on cache prefetch mode */                              
  PREFETCH_ENABLE();                                             
  LED1_INIT();													 
  LED2_INIT();			
  INIT_PUSH1();
  INIT_PUSH2();
                                                                 
  /* configure tristates */                                      
  P0INP |= PUSH2_BV;                                           
}