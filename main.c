#include <time.h>
#include <stdlib.h>

#include "xgpio.h"
#include "xparameters.h"
#include "xil_exception.h"
#include "xscugic.h"

#include "PmodKYPD.h"
#include "sleep.h"

#include "PmodGPIO.h"
#include "sleep.h"
#include "xil_printf.h"

//Parameters
#define INTC_DEVICE_ID 		XPAR_PS7_SCUGIC_0_DEVICE_ID
#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID
#define INTC_GPIO_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define BTN_INT 			XGPIO_IR_CH1_MASK


PmodKYPD myDevice;

PmodGPIO myDevice1;
PmodGPIO myDevice2;
PmodGPIO myDevice3;

XGpio BTNInst;
XScuGic INTCInst;
static int btn_value;





void DemoInitialize();
void DemoRun();
static void BTN_Intr_Handler(void *baseaddr_p);
static int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
static int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr);
static int data[4][16] = {
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
		{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
static int lastBtn;
static int btnLayer = 1;


int main() {
   DemoInitialize();
   DemoRun();
   return 0;
}

void display(int data[][16]){

	for(int i = 0; i < 4; i++){
		for(int j = 0; j < 16; j++){
			if(j < 8){
				GPIO_setPin(&myDevice2, j + 1, data[i][j]);
			}else{
				GPIO_setPin(&myDevice3, j - 8 + 1, data[i][j]);
			}
		}
		GPIO_setPin(&myDevice1, i + 1, 0);
		usleep(1000);
		GPIO_setPins(&myDevice1, 0xFF);
		GPIO_setPins(&myDevice2, 0x00);
		GPIO_setPins(&myDevice3, 0x00);
	}
}


void cube(){
 	GPIO_setPins(&myDevice2, 0x60);
    GPIO_setPins(&myDevice3, 0x06);
    GPIO_setPins(&myDevice1, ~0x06);
    usleep(200000);
    for(int i = 0; i < 200;i++){
    	GPIO_setPins(&myDevice2, 0x9F);
    	GPIO_setPins(&myDevice3, 0xF9);
    	GPIO_setPins(&myDevice1, ~0x09);
    	usleep(1000);
    	GPIO_setPins(&myDevice2, 0x09);
    	GPIO_setPins(&myDevice3, 0x90);
    	GPIO_setPins(&myDevice1, ~0x06);
    	usleep(1000);
    }
}

void sequence(){
	GPIO_setPins(&myDevice1, 0xFF);
	for(int j = 1; j<=4; j++){

			GPIO_setPin(&myDevice1, j , 0);

			   for(int i = 1; i < 17;i++){
				   if(i<9){
					    GPIO_setPin(&myDevice2, i , 1);
					   	xil_printf("Pin: %d\n\r",i);
					   	usleep(100000);
					   	GPIO_setPin(&myDevice2, i , 0);
				   }else{
					   GPIO_setPin(&myDevice3, i-8 , 1);
					   xil_printf("Pin: %d\n\r",i);
					   usleep(100000);
					   GPIO_setPin(&myDevice3, i-8 , 0);
				   }
			   }
			   GPIO_setPin(&myDevice1, j , 1);
		   }
}

void randomLED(){

	GPIO_setPins(&myDevice1, 0xFF);
	int rnumTime = (rand() % 100) * 10000;
	      int rnum1 = (rand() % 4) + 1;
	      GPIO_setPin(&myDevice1, rnum1 , 0);
	      int rnum2 = (rand() % 16) + 1;
	      if(rnum2 <=8){
	    	  GPIO_setPin(&myDevice2, rnum2 , 1);
	    	  usleep(rnumTime);
	    	  GPIO_setPin(&myDevice2, rnum2 , 0);
	      }else{
	    	  GPIO_setPin(&myDevice3, rnum2 - 8 , 1);
	    	  usleep(rnumTime);
	    	  GPIO_setPin(&myDevice3, rnum2 - 8 , 0);
	      }
	      GPIO_setPin(&myDevice1, rnum1 , 1);

}
void cornerCubes(){
	GPIO_setPins(&myDevice1, ~0x01);
	GPIO_setPins(&myDevice2, 0x00);
	GPIO_setPins(&myDevice3, 0x80);
	usleep(2000000);
	GPIO_setPins(&myDevice1, ~0x03);
	GPIO_setPins(&myDevice2, 0x00);
	GPIO_setPins(&myDevice3, 0xCC);
	usleep(2000000);
	GPIO_setPins(&myDevice1, ~0x07);
	GPIO_setPins(&myDevice2, 0xE0);
	GPIO_setPins(&myDevice3, 0xEE);
	usleep(2000000);
	GPIO_setPins(&myDevice1, ~0x0F);
	GPIO_setPins(&myDevice2, 0xFF);
	GPIO_setPins(&myDevice3, 0xFF);
	usleep(2000000);
	GPIO_setPins(&myDevice1, ~0x0E);
	GPIO_setPins(&myDevice2, 0x77);
	GPIO_setPins(&myDevice3, 0x07);
	usleep(2000000);
	GPIO_setPins(&myDevice1, ~0x0C);
	GPIO_setPins(&myDevice2, 0x33);
	GPIO_setPins(&myDevice3, 0x00);
	usleep(2000000);
	GPIO_setPins(&myDevice1, ~0x08);
	GPIO_setPins(&myDevice2, 0x01);
	GPIO_setPins(&myDevice3, 0x00);
	usleep(2000000);

}

void DemoInitialize() {
   GPIO_begin(&myDevice1, XPAR_PMODGPIO_0_AXI_LITE_GPIO_BASEADDR, 0x00);
   GPIO_begin(&myDevice2, XPAR_PMODGPIO_1_AXI_LITE_GPIO_BASEADDR, 0x00);
   GPIO_begin(&myDevice3, XPAR_PMODGPIO_2_AXI_LITE_GPIO_BASEADDR, 0x00);
   KYPD_begin(&myDevice, XPAR_PMODKYPD_0_AXI_LITE_GPIO_BASEADDR);
}

void DemoRun() {
   u8 count = 0;
   int i = 0;
   int status;

   u16 keystate;
   XStatus statusKYPD, last_status = KYPD_NO_KEY;
   u8 key, last_key = 'x';


   // Initialise Push Buttons
   status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
   if(status != XST_SUCCESS) return XST_FAILURE;
   // Set all buttons direction to inputs
   XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

   // Initialize interrupt controller
   status = IntcInitFunction(INTC_DEVICE_ID, &BTNInst);
   if(status != XST_SUCCESS) return XST_FAILURE;

   xil_printf("GPIO Output Demo\n\r");
   GPIO_setPins(&myDevice1, 0xFF);


   while(1){
	   switch(btn_value){
	   case 1:

		   // Capture state of each key
		   keystate = KYPD_getKeyStates(&myDevice);

		   // Determine which single key is pressed, if any
		   status = KYPD_getKeyPressed(&myDevice, keystate, &key);

		   // Print key detect if a new key is pressed or if status has changed
		   if (status == KYPD_SINGLE_KEY
		         && (status != last_status || key != last_key)) {
		      xil_printf("Key Pressed: %d\r\n",key);
		      data[btnLayer - 1][key] = ~data[btnLayer - 1][key];


		      last_key = key;
		   } else if (status == KYPD_MULTI_KEY && status != last_status)
		      xil_printf("Error: Multiple keys pressed\r\n");

		   last_status = status;
		   display(data);
	   	   break;
	   case 2:
	   	   sequence();
	   	   break;
	   case 4:
		   randomLED();
	   	   break;
	   case 8:
		   cornerCubes();
	   	   break;
	   }
   }

}

void BTN_Intr_Handler(void *InstancePtr)
{
	// Disable GPIO interrupts
	XGpio_InterruptDisable(&BTNInst, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(&BTNInst) & BTN_INT) != BTN_INT) {
			return;
		}
	if(XGpio_DiscreteRead(&BTNInst, 1) != 0){
		btn_value = XGpio_DiscreteRead(&BTNInst, 1);
		xil_printf("Button pressed is: %d\n\r",btn_value);
		if(btn_value == 1){
			btnLayer++;
			if(btnLayer >= 5 || btn_value != lastBtn){
				btnLayer = 1;
			}
		}
		xil_printf("Button pressed is: %d\n\r",btnLayer);
	}
	lastBtn = btn_value;
	(void)XGpio_InterruptClear(&BTNInst, BTN_INT);
    // Enable GPIO interrupts
    XGpio_InterruptEnable(&BTNInst, BTN_INT);
    //DemoRun();
}

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr)
{
	// Enable interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 	 	 	 	 	 (Xil_ExceptionHandler)XScuGic_InterruptHandler,
			 	 	 	 	 	 XScuGicInstancePtr);
	Xil_ExceptionEnable();


	return XST_SUCCESS;

}

int IntcInitFunction(u16 DeviceId, XGpio *GpioInstancePtr)
{
	XScuGic_Config *IntcConfig;
	int status;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig, IntcConfig->CpuBaseAddress);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
					  	  	 INTC_GPIO_INTERRUPT_ID,
					  	  	 (Xil_ExceptionHandler)BTN_Intr_Handler,
					  	  	 (void *)GpioInstancePtr);
	if(status != XST_SUCCESS) return XST_FAILURE;

	// Enable GPIO interrupts interrupt
	XGpio_InterruptEnable(GpioInstancePtr, 1);
	XGpio_InterruptGlobalEnable(GpioInstancePtr);

	// Enable GPIO and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_GPIO_INTERRUPT_ID);

	return XST_SUCCESS;
}
