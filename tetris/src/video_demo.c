/* ------------------------------------------------------------ */
/*				Include File Definitions						*/
/* ------------------------------------------------------------ */
#include "video_demo.h"
#include "video_capture/video_capture.h"
#include "display_ctrl/display_ctrl.h"
#include "intc/intc.h"
#include <stdio.h>
#include "xuartps.h"
#include "math.h"
#include <ctype.h>
#include <stdlib.h>
#include "xil_types.h"
#include "xil_cache.h"
#include "timer_ps/timer_ps.h"
#include "xparameters.h"
#include "xtmrctr.h"
#include "xtime_l.h"
/*
 * XPAR redefines
 */
#define DYNCLK_BASEADDR XPAR_AXI_DYNCLK_0_BASEADDR
#define VGA_VDMA_ID XPAR_AXIVDMA_0_DEVICE_ID
#define DISP_VTC_ID XPAR_VTC_0_DEVICE_ID
#define VID_VTC_ID XPAR_VTC_1_DEVICE_ID
#define VID_GPIO_ID XPAR_AXI_GPIO_VIDEO_DEVICE_ID
#define VID_VTC_IRPT_ID XPS_FPGA3_INT_ID
#define VID_GPIO_IRPT_ID XPS_FPGA4_INT_ID
#define SCU_TIMER_ID XPAR_SCUTIMER_DEVICE_ID
#define UART_BASEADDR XPAR_PS7_UART_1_BASEADDR

#define INTC_BTN_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_BTN_IP2INTC_IRPT_INTR
#define INTC_KNAP_INTERRUPT_ID XPAR_FABRIC_AXI_GPIO_0_IP2INTC_IRPT_INTR
#define INTC_TMR_INTERRUPT_ID XPAR_FABRIC_AXI_TIMER_0_INTERRUPT_INTR

#define BTNS_DEVICE_ID		XPAR_AXI_GPIO_BTN_DEVICE_ID
#define KNAP_DEVICE_ID		XPAR_AXI_GPIO_0_DEVICE_ID
#define TMR_DEVICE_ID		XPAR_TMRCTR_0_DEVICE_ID

#define BTN_INT 			XGPIO_IR_CH1_MASK
#define KNAP_INT 			XGPIO_IR_CH1_MASK
#define TMR_LOAD			45000000

XGpio BTNInst, KNAPInst;
XScuGic INTCInst;

XTmrCtr TMRInst;
XTime tStart, tEnd;
/* ------------------------------------------------------------ */
/*				Global Variables								*/
/* ------------------------------------------------------------ */
int btn_value=0;
int knap_value = 0;
int space[16][10];
int space_y;
int space_x;
double TMR_LOAD1 = 45000000;
double TMR_LOAD2;
//Self determined variables
double backgroundRed = 128.0;
double backgroundBlue = 128.0;
double backgroundGreen = 128.0;
double sRed = 0;
double sBlue = 0;
double sGreen = 0;
int pos = 0;
int tempMove = 0;
int tempDown = 0;
int randBlock = 0;
int randBlock2 = 0;
int randBlock3 = 1;
int newBlock = 0;
int start = 0;
char userInput = 0;
int pointScore = 0;
int HighScore[5];
int limit = 0;
int levelNum = 1;
int temptime;
int dropcheck;
u32 cxcoi, cycoi, cxcoi2, cycoi2;
u32 cwidth, cheight, cwidth2, cheight2;
/*
 * Display and Video Driver structs
 */
DisplayCtrl dispCtrl;
XAxiVdma vdma;
INTC intc;
char fRefresh; //flag used to trigger a refresh of the Menu on video detect

/*
 * Framebuffers for video data
 */
u8 frameBuf[DISPLAY_NUM_FRAMES][DEMO_MAX_FRAME];
u8 *pFrames[DISPLAY_NUM_FRAMES]; //array of pointers to the frame buffers


/* ------------------------------------------------------------ */
/*				Procedure Definitions							*/
/* ------------------------------------------------------------ */

//prototype
void BTN_Intr_Handler(void *baseaddr_p);
void KNAP_Intr_Handler(void *baseaddr_p);
void TMR_Intr_Handler(void *InstancePtr, u8 TmrCtrNumber);
int InterruptSystemSetup(XScuGic *XScuGicInstancePtr);
int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr,
		XGpio *BtnInstancePtr, XGpio *KnapInstancePtr);

// for at bruge interrupt, i forhold til flag
void XTmrCtr_ClearInterruptFlag(XTmrCtr * InstancePtr, u8 TmrCtrNumber) {
	u32 CounterControlReg;

	Xil_AssertVoid(InstancePtr != NULL);
	Xil_AssertVoid(TmrCtrNumber < XTC_DEVICE_TIMER_COUNT);
	Xil_AssertVoid(InstancePtr->IsReady == XIL_COMPONENT_IS_READY);

	/*
	 * Read current contents of the CSR register so it won't be destroyed
	 */
	CounterControlReg = XTmrCtr_ReadReg(InstancePtr->BaseAddress, TmrCtrNumber,
			XTC_TCSR_OFFSET);
	/*
	 * Reset the interrupt flag
	 */
	XTmrCtr_WriteReg(InstancePtr->BaseAddress, TmrCtrNumber, XTC_TCSR_OFFSET,
			CounterControlReg | XTC_CSR_INT_OCCURED_MASK);
}

void BTN_Intr_Handler(void *InstancePtr) {
	//xil_printf("BTN Interrupt handler\n");
	// Disable GPIO interrupts
	XGpio_InterruptDisable(InstancePtr, BTN_INT);
	// Ignore additional button presses
	if ((XGpio_InterruptGetStatus(InstancePtr) & BTN_INT) !=
	BTN_INT) {
		return;
	}
	btn_value = XGpio_DiscreteRead(InstancePtr, 1);
	// Increment counter based on button value
	// Reset if centre button pressed
	//led_data = led_data + btn_value;
	int check=1;

	int counter = 0;

	switch(btn_value){
	//---------------------------------------
	//*********** NÅR BTN SLIPPER ***********
	//---------------------------------------
	case 0:

			break;
	//----------------------------------------
	//***************BTN 1, BTN A***********
	//----------------------------------------
	case 1:
		//rotate
		if(start == 1){
			Rotate(randBlock3);
		}
		break;
	//----------------------------------------
	//***************BTN 2, NOT USED***********
	//----------------------------------------
	case 2:
		if (start == 0){
			start = 1;
			pos = 0;
			tempMove = 0;
			tempDown = -30;
			limit = 500;
			TMR_LOAD1 = 45000000;
			XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
			levelNum = 1;
			pointScore = 0;
			Background(pFrames[dispCtrl.curFrame], dispCtrl.vMode.width,
					dispCtrl.vMode.height, DEMO_STRIDE);
			gameField(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			memset(space, 0, sizeof(space[0][0]) * 15 * 10);
		}
		break;
	//----------------------------------------
	//***************BTN 4, BTN C***********
	//----------------------------------------
	case 4:
		while (btn_value==4){
			btn_value = XGpio_DiscreteRead(InstancePtr, 1);
				//enter here

				counter++;
				if(counter>=900000){
				//xil_printf("der er trykket på knap 3\n");
					if (check==400000){
						if (start == 1) {
							shift_right(randBlock3);
						}
						check=1;
					}else {
						check++;
					}

				}
			}
		if(counter<100000000){
		//xil_printf("der er trykket på knap 3\n");
			if (start == 1) {
				shift_right(randBlock3);
			}
		}
		//shift right
		break;
	//----------------------------------------
	//***************BTN 8, BTN L***********
	//----------------------------------------
	case 8:
		while (btn_value==8){
			btn_value = XGpio_DiscreteRead(InstancePtr, 1);
				counter++;
				if(counter>=900000){
				//xil_printf("der er trykket på knap 4\n");
					if (check==400000){
						if (start == 1) {
							shift_left(randBlock3);
						}
						check=1;
					}else {
						check++;
					}
				}
			}
		if(counter<100000000){
		//xil_printf("der er trykket på knap 4\n");
			if (start == 1) {
				shift_left(randBlock3);
			}
		}
		//shift left
		break;
	default:
	xil_printf("fejl");
	}
	XGpio_InterruptClear(InstancePtr, BTN_INT);
	XGpio_InterruptEnable(InstancePtr, BTN_INT);
}
void KNAP_Intr_Handler(void *InstancePtr) {
		// Disable GPIO interrupts
		XGpio_InterruptDisable(InstancePtr, KNAP_INT);
		// Ignore additional button presses
		if ((XGpio_InterruptGetStatus(InstancePtr) & KNAP_INT) !=
				KNAP_INT) {
			return;
		}
		knap_value = XGpio_DiscreteRead(InstancePtr, 1);
		// Increment counter based on button value
		// Reset if centre button pressed
		int check=1;

		int counter = 0;

		//xil_printf("Knap: %d\n", knap_value);
		switch(knap_value){
		//---------------------------------------
		//*********** NÅR BTN SLIPPER ***********
		//---------------------------------------
		case 224:

				break;
		//----------------------------------------
		//***************Knap 5, ROTATE**********
		//----------------------------------------
		case 226:
			//rotate
			if(start == 1){
				while (knap_value==226){
							knap_value = XGpio_DiscreteRead(InstancePtr, 1);
				}
				check=0;
				Rotate(randBlock3);
			}
			break;
		//----------------------------------------
		//***************Knap 4, DROP ***********
		//----------------------------------------
		case 225:
			if(start == 1){
				while (knap_value == 225) {
					knap_value = XGpio_DiscreteRead(InstancePtr, 1);
				}
				temptime=10000000;
				//TMR_LOAD2 = TMR_LOAD1;
				XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD /temptime);
				dropcheck=1;
			}
			break;
		//----------------------------------------
		//***************Knap2, SHIFT RIGHT ***********
		//----------------------------------------
		case 228:
			while (knap_value==228){
				knap_value = XGpio_DiscreteRead(InstancePtr, 1);
					//enter here

					counter++;
					if(counter>=900000){
						if (check==400000){
							if (start == 1) {
								shift_right(randBlock3);
							}
							check=1;
						}else {
							check++;
						}

					}
				}
			if(counter<100000000){
				if (start == 1) {
					shift_right(randBlock3);
				}
			}
			//shift right
			break;
		//----------------------------------------
		//***************Knap 1 SHIFT LEFT***********
		//----------------------------------------
		case 232:
			while (knap_value==232){
				knap_value = XGpio_DiscreteRead(InstancePtr, 1);
					counter++;
					if(counter>=900000){
						if (check==400000){
							if (start == 1) {
								shift_left(randBlock3);
							}
							check=1;
						}else {
							check++;
						}
					}
				}
			if(counter<100000000){
				if (start == 1) {
					shift_left(randBlock3);
				}
			}
			//shift left
			break;
		//START GAME
		case 240:
		if (start == 0) {
			start = 1;
			pos = 0;
			tempMove = 0;
			tempDown = -30;
			limit = 500;
			TMR_LOAD1 = 45000000;
			XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
			levelNum = 1;
			pointScore = 0;
			Background(pFrames[dispCtrl.curFrame], dispCtrl.vMode.width,
					dispCtrl.vMode.height, DEMO_STRIDE);
			gameField(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			memset(space, 0, sizeof(space[0][0]) * 15 * 10);
		}
			break;
		default:
		xil_printf("fejl");
		}
		XGpio_InterruptClear(InstancePtr, KNAP_INT);
		XGpio_InterruptEnable(InstancePtr, KNAP_INT);
}

void TMR_Intr_Handler(void *InstancePtr, u8 TmrCtrNumber) {
	if (start==0){
		XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
	}
	double duration;
	static int tmr_count;
	XTime_GetTime(&tEnd);
	XTmrCtr* pTMRInst = (XTmrCtr *) InstancePtr;

	//xil_printf("Timer %d interrupt \n", TmrCtrNumber);

	if (TmrCtrNumber == 0) { //Handle interrupts generated by timer 0
		duration = ((double) (tEnd - tStart)) / COUNTS_PER_SECOND;
		//xil_printf("hej\n");
		//xil_printf("Tmr_interrupt, tmr_count= %d, duration=%.6f s\n\r", tmr_count, (double)duration);

		tStart = tEnd;

		if (XTmrCtr_IsExpired(pTMRInst, 0)) {
			// Once timer has expired 3 times, stop, increment counter
			// reset timer and start running again
			if (tmr_count == 0) {
				if(start == 0){
					nextBlock(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				}
				StopGame();
				if(start == 1){
					if (newBlock == 0) {
						clear(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
					}
					drop(randBlock3);
					if (newBlock == 1) {
						change_block();
					}
					if(pointScore > limit){
						TMR_LOAD1=TMR_LOAD1*0.9;
						XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
						levelNum = levelNum + 1;
						limit = limit + 500;
						levelNumber(levelNum);
					}
				}

				XTmrCtr_Stop(pTMRInst, 0);

				tmr_count = 0;

				//XGpio_DiscreteWrite(&LEDInst, 1, led_data);
				XTmrCtr_Reset(pTMRInst, 0);
				XTmrCtr_Start(pTMRInst, 0);

			} else
				tmr_count++;
		}
	} else {  //Handle interrupts generated by timer 1

	}

	XTmrCtr_ClearInterruptFlag(pTMRInst, TmrCtrNumber);
}

int main(void)
{
	for(int startSpace = 0; startSpace < 10; startSpace++){
		space[15][startSpace] = 9;
	}
	xil_printf("start\n");
	int status;
	// Initialise Push Buttons
	status = XGpio_Initialize(&BTNInst, BTNS_DEVICE_ID);
		if (status != XST_SUCCESS)
			return XST_FAILURE;
		// Set all buttons direction to inputs
			XGpio_SetDataDirection(&BTNInst, 1, 0xFF);

	status = XGpio_Initialize(&KNAPInst, KNAP_DEVICE_ID);
	if (status != XST_SUCCESS)
		return XST_FAILURE;
	// Set all buttons direction to inputs
	XGpio_SetDataDirection(&KNAPInst, 1, 0xFF);

	status = XTmrCtr_Initialize(&TMRInst, TMR_DEVICE_ID);
	if (status != XST_SUCCESS)
	return XST_FAILURE;
	XTmrCtr_SetHandler(&TMRInst, TMR_Intr_Handler, &TMRInst);
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
	XTmrCtr_SetOptions(&TMRInst, 0,
	XTC_INT_MODE_OPTION | XTC_AUTO_RELOAD_OPTION | XTC_DOWN_COUNT_OPTION);

	// Initialize interrupt controller
	status = IntcInitFunction(INTC_DEVICE_ID, &TMRInst, &BTNInst, &KNAPInst);
	if (status != XST_SUCCESS)
	return XST_FAILURE;

	XTmrCtr_Start(&TMRInst, 0);
	//Here we get the time when the timer first started
	XTime_GetTime(&tStart);
	randBlock = rand() % 7;
	InitializeGame();
	Run();
		while(1){

		}

	return 0;
}

int InterruptSystemSetup(XScuGic *XScuGicInstancePtr) {
	// Enable interrupt
	XGpio_InterruptEnable(&BTNInst, BTN_INT);
	XGpio_InterruptGlobalEnable(&BTNInst);

	//switches

	XGpio_InterruptEnable(&KNAPInst, KNAP_INT);
	XGpio_InterruptGlobalEnable(&KNAPInst);

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			(Xil_ExceptionHandler) XScuGic_InterruptHandler,
			XScuGicInstancePtr);
	Xil_ExceptionEnable();

	return XST_SUCCESS;

}

int IntcInitFunction(u16 DeviceId, XTmrCtr *TmrInstancePtr,
		XGpio *BtnInstancePtr, XGpio *KnapInstancePtr) {
	XScuGic_Config *IntcConfig;
	int status;
	u8 pri, trig;

	// Interrupt controller initialisation
	IntcConfig = XScuGic_LookupConfig(DeviceId);
	status = XScuGic_CfgInitialize(&INTCInst, IntcConfig,
			IntcConfig->CpuBaseAddress);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	// Call to interrupt setup
	status = InterruptSystemSetup(&INTCInst);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	// Connect GPIO interrupt to handler
	status = XScuGic_Connect(&INTCInst,
	INTC_BTN_INTERRUPT_ID, (Xil_InterruptHandler) BTN_Intr_Handler,
			(void *) BtnInstancePtr);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	// Connect GPIO interrupt switches

	status = XScuGic_Connect(&INTCInst,
	INTC_KNAP_INTERRUPT_ID, (Xil_InterruptHandler) KNAP_Intr_Handler,
			(void *) KnapInstancePtr);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	// Connect timer interrupt to handler
	status = XScuGic_Connect(&INTCInst,
	INTC_TMR_INTERRUPT_ID,
			// (Xil_ExceptionHandler)TMR_Intr_Handler,
			(Xil_ExceptionHandler) XTmrCtr_InterruptHandler,
			(void *) TmrInstancePtr);
	if (status != XST_SUCCESS)
		return XST_FAILURE;

	// Enable BTN interrupts interrupt
	XGpio_InterruptEnable(BtnInstancePtr, 1);
	XGpio_InterruptGlobalEnable(BtnInstancePtr);

	// Enable SW interrupts interrupt

	XGpio_InterruptEnable(KnapInstancePtr, 1);
	XGpio_InterruptGlobalEnable(KnapInstancePtr);

	// Enable sws, btn and timer interrupts in the controller
	XScuGic_Enable(&INTCInst, INTC_BTN_INTERRUPT_ID);
	XScuGic_Enable(&INTCInst, INTC_KNAP_INTERRUPT_ID);
	XScuGic_Enable(&INTCInst, INTC_TMR_INTERRUPT_ID);

	xil_printf("Getting the Timer interrupt info\n\r");
	XScuGic_GetPriTrigTypeByDistAddr(INTCInst.Config->DistBaseAddress,
	INTC_TMR_INTERRUPT_ID, &pri, &trig);
	xil_printf("GPIO Interrupt-> Priority:%d, Trigger:%x\n\r", pri, trig);

	//Set the timer interrupt as edge triggered
	//XScuGic_SetPriorityTriggerType(&INTCInst, INTC_TMR_INTERRUPT_ID, )

	return XST_SUCCESS;
}

void InitializeGame()
{
	int Status;
	XAxiVdma_Config *vdmaConfig;
	int i;

	/*
	 * Initialize an array of pointers to the 3 frame buffers
	 */
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++)
	{
		pFrames[i] = frameBuf[i];
	}
	/*
	 * Initialize a timer used for a simple delay
	 */
	TimerInitialize(SCU_TIMER_ID);
	/*
	 * Initialize VDMA driver
	 */
	vdmaConfig = XAxiVdma_LookupConfig(VGA_VDMA_ID);
	if (!vdmaConfig)
	{
		xil_printf("No video DMA found for ID %d\r\n", VGA_VDMA_ID);
		return;
	}
	Status = XAxiVdma_CfgInitialize(&vdma, vdmaConfig, vdmaConfig->BaseAddress);
	if (Status != XST_SUCCESS)
	{
		xil_printf("VDMA Configuration Initialization failed %d\r\n", Status);
		return;
	}

	/*
	 * Initialize the Display controller and start it
	 */
	Status = DisplayInitialize(&dispCtrl, &vdma, DISP_VTC_ID, DYNCLK_BASEADDR, pFrames, DEMO_STRIDE);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Display Ctrl initialization failed during demo initialization%d\r\n", Status);
		return;
	}
	Status = DisplayStart(&dispCtrl);
	if (Status != XST_SUCCESS)
	{
		xil_printf("Couldn't start display during demo initialization%d\r\n", Status);
		return;
	}
	//Initialize background
	Menu(dispCtrl.framePtr[dispCtrl.curFrame], dispCtrl.vMode.width, dispCtrl.vMode.height, dispCtrl.stride);

	return;
}

void Run()
{
	// Flush UART FIFO
	while (XUartPs_IsReceiveData(UART_BASEADDR))
	{
		XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);
	}

	while (userInput != 'q')
	{
		fRefresh = 0;
		PrintMenu();

		//Wait for data on UART

		while (!XUartPs_IsReceiveData(UART_BASEADDR) && !fRefresh)
		{}

		// Store the first character in the UART receive FIFO and echo it
		if (XUartPs_IsReceiveData(UART_BASEADDR))
		{
			userInput = XUartPs_ReadReg(UART_BASEADDR, XUARTPS_FIFO_OFFSET);
			xil_printf("%c", userInput);
		}
		else  //Refresh triggered by video detect interrupt
		{
			userInput = 'r';
		}

		switch (userInput)
		{
		case '2':
			if (start == 0){
				start = 1;
				pos = 0;
				tempMove = 0;
				tempDown = -30;
				limit = 500;
				levelNum = 1;
				pointScore = 0;
				TMR_LOAD1 = 45000000;
				XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD);
				Background(pFrames[dispCtrl.curFrame], dispCtrl.vMode.width,
						dispCtrl.vMode.height, DEMO_STRIDE);
				gameField(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				memset(space, 0, sizeof(space[0][0]) * 15 * 10);
			}
			break;
		case '4':
			if(start == 1){
				shift_left(randBlock3);
			}
			break;
		case '5':
			if(start == 1){
				Rotate(randBlock3);
			}
			break;
		case '6':
			if (start == 1) {
				shift_right(randBlock3);
			}
			break;
		case '8':
			if(start == 1){
				temptime = 10000000;
				TMR_LOAD2 = TMR_LOAD1;
				XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD / temptime);
				dropcheck = 1;
			}
			break;
		case 'q':
			break;
		case 'r':
			break;
		default :
			xil_printf("\n\rInvalid Selection");
			TimerDelay(500000);
		}
	}
	return;
}

void PrintMenu()
{
	xil_printf("\x1B[H"); //Set cursor to top left of terminal
	xil_printf("\x1B[2J"); //Clear terminal
	xil_printf("**************************************************\n\r");
	xil_printf("*                     TETRIS                     *\n\r");
	xil_printf("**************************************************\n\r");
	xil_printf("*Display Resolution: %28s*\n\r", dispCtrl.vMode.label);
	printf("*Display Pixel Clock Freq. (MHz): %15.3f*\n\r", dispCtrl.pxlFreq);
	xil_printf("**************************************************\n\r");
	xil_printf("\n\r");
	xil_printf("Controls: \n\r");
	xil_printf("2 - Start game! \n\r\n\r");
	xil_printf("5 - Rotate block\n\r");
	xil_printf("4 - Move left & 6 - Move right\n\r");
	xil_printf("8 - Drop\n\r");
	xil_printf("\n\r");
	xil_printf("\n\r");
	xil_printf("Enter a selection:");
}
void Rotate(int block){
	array(tempMove, tempDown);
	if (randBlock3 != 1) {
		clear(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
	}
	if ((block == 0 || block == 7) && pos == 0) {

		if (tempMove == 180) {

			if (space[space_y][space_x - 1] != 0
					|| space[space_y][space_x - 2] != 0
					|| space[space_y][space_x - 3] != 0) {
				I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			} else {
				I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				pos = 1;
			}
		} else if (tempMove == 150) {

			if (space[space_y][space_x + 1] != 0
					|| space[space_y][space_x - 2] != 0
					|| space[space_y][space_x - 1] != 0) {
				I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			} else {
				I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				pos = 1;
			}
		} else if (tempMove == 120) {

			if (space[space_y][space_x - 1] != 0
					|| space[space_y][space_x + 2] != 0
					|| space[space_y][space_x + 1] != 0) {
				I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			} else {
				I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				pos = 1;
			}
		} else {

			if (space[space_y + 1][space_x] != 0
					|| space[space_y + 2][space_x] != 0
					|| space[space_y + 3][space_x] != 0) {
				I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			} else {
				I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
				pos = 1;
			}
		}

	} else if ((block == 0 || block == 7) && pos == 1) {
		if (space[space_y + 1][space_x] != 0 || space[space_y + 2][space_x] != 0
				|| space[space_y + 3][space_x] != 0) {
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			pos = 0;
		}
	}
	if (block == 2 && pos == 0) {
		if (space[space_y][space_x] != 0 || space[space_y + 2][space_x] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 1;
		}

	} else if (block == 2 && pos == 1) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y][space_x + 2] != 0) {
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 2;
		}
	} else if (block == 2 && pos == 2) {
		if (space[space_y + 1][space_x + 1] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 3;
		}
	} else if (block == 2 && pos == 3) {

		if (space[space_y + 1][space_x] != 0
				|| space[space_y + 1][space_x + 2] != 0
				|| space[space_y][space_x + 2] != 0) {
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			pos = 0;
		}
	}
	if (block == 3 && pos == 0) {

		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 2][space_x] != 0) {
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 1;
		}
	} else if (block == 3 && pos == 1) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 2] != 0) {
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 2;
		}
	} else if (block == 3 && pos == 2) {
		if (space[space_y + 1][space_x + 1] != 0
				|| space[space_y + 2][space_x] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 3;
		}
	} else if (block == 3 && pos == 3) {
		if (space[space_y][space_x] != 0 || space[space_y + 1][space_x] != 0
				|| space[space_y + 1][space_x + 2] != 0) {
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			pos = 0;
		}
	}
	if (block == 4 && pos == 0) {
		if (space[space_y][space_x] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 1;
		}
	} else if (block == 4 && pos == 1) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y][space_x + 2] != 0) {
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			pos = 0;
		}
	}
	if (block == 5 && pos == 0) {
		if (space[space_y + 1][space_x] != 0
				|| space[space_y + 2][space_x] != 0) {
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 1;
		}
	} else if (block == 5 && pos == 1) {
		if (space[space_y][space_x] != 0
				|| space[space_y + 1][space_x + 2] != 0) {
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			pos = 0;
		}
	}
	if (block == 6 && pos == 0) {
		if (space[space_y][space_x] != 0 || space[space_y + 2][space_x] != 0) {
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 1;
		}
	} else if (block == 6 && pos == 1) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y][space_x + 2] != 0) {
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 2;
		}
	} else if (block == 6 && pos == 2) {
		if (space[space_y + 1][space_x + 2] != 0
				|| space[space_y + 2][space_x + 2] != 0) {
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			pos = 3;
		}
	} else if (block == 6 && pos == 3) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x] != 0) {
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			pos = 0;
		}
	}
}
void shift_left(int block){
	array(tempMove, tempDown);
	clear(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
	//Blok I
	if ((block == 0 || block == 7) && pos == 0) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0
				|| space[space_y + 2][space_x - 1] != 0
				|| space[space_y + 3][space_x - 1] != 0) {
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if ((block == 0 || block == 7) && pos == 1) {
		if (space[space_y][space_x - 1] != 0) {
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//BLok O
	if (block == 1) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0) {
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	}
	//Blok L
	if (block == 2 && pos == 0) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x - 1] != 0) {
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 2 && pos == 1) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0
				|| space[space_y + 2][space_x - 1] != 0) {
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 2 && pos == 2) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0) {
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 2 && pos == 3) {
		if (space[space_y][space_x - 1] != 0 || space[space_y + 1][space_x] != 0
				|| space[space_y + 2][space_x] != 0) {
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//Blok J
	if (block == 3 && pos == 0) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0) {
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if (block == 3 && pos == 1) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0
				|| space[space_y + 2][space_x - 1] != 0) {
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 3 && pos == 2) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x + 1] != 0) {
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 3 && pos == 3) {
		if (space[space_y][space_x] != 0 || space[space_y + 1][space_x] != 0
				|| space[space_y + 2][space_x - 1] != 0) {
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
	//blok S
	if (block == 4 && pos == 0) {
		if (space[space_y][space_x] != 0
				|| space[space_y + 1][space_x - 1] != 0) {
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 4 && pos == 1) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0
				|| space[space_y + 2][space_x] != 0) {
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
	//Blok Z
	if (block == 5 && pos == 0) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x] != 0) {
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 5 && pos == 1) {
		if (space[space_y][space_x] != 0 || space[space_y + 1][space_x - 1] != 0
				|| space[space_y + 2][space_x - 1] != 0) {
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
	//Blok T
	if (block == 6 && pos == 0) {
		if (space[space_y][space_x] != 0
				|| space[space_y + 1][space_x - 1] != 0) {
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove - 30;
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 6 && pos == 1) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x - 1] != 0
				|| space[space_y + 2][space_x - 1] != 0) {
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 6 && pos == 2) {
		if (space[space_y][space_x - 1] != 0
				|| space[space_y + 1][space_x] != 0) {
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 6 && pos == 3) {
		if (space[space_y][space_x + 1] != 0 || space[space_y + 1][space_x] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove - 30;
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
}
void shift_right(int block){
	array(tempMove, tempDown);

	clear(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
	//Blok I

	if ((block == 0 || block == 7) && pos == 0) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x + 1] != 0
				|| space[space_y + 2][space_x + 1] != 0
				|| space[space_y + 3][space_x + 1] != 0) {
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if ((block == 0 || block == 7) && pos == 1) {
		if (space[space_y][space_x + 4] != 0) {
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
	//Blok O
	if (block == 1) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 2] != 0) {
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	}
	//Blok L
	if (block == 2 && pos == 0) {
		if (space[space_y][space_x + 3] != 0
				|| space[space_y + 1][space_x + 3] != 0) {
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 2 && pos == 1) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x + 1] != 0
				|| space[space_y + 2][space_x + 2] != 0) {
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 2 && pos == 2) {
		if (space[space_y][space_x + 3] != 0
				|| space[space_y + 1][space_x + 1] != 0) {
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 2 && pos == 3) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 2] != 0
				|| space[space_y + 2][space_x + 2] != 0) {
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
	//Blok J
	if (block == 3 && pos == 0) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x + 3] != 0) {
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if (block == 3 && pos == 1) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 1] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 3 && pos == 2) {
		if (space[space_y][space_x + 3] != 0
				|| space[space_y + 1][space_x + 1] != 0) {
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 3 && pos == 3) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 2] != 0
				|| space[space_y + 2][space_x + 2] != 0) {
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}

	//Blok S
	if (block == 4 && pos == 0) {
		if (space[space_y][space_x + 3] != 0
				|| space[space_y + 1][space_x + 2] != 0) {
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 4 && pos == 1) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x + 2] != 0
				|| space[space_y + 2][space_x + 2] != 0) {
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//Blok Z
	if (block == 5 && pos == 0) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 3] != 0) {
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 5 && pos == 1) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 2] != 0
				|| space[space_y + 2][space_x + 11] != 0) {
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	}
	if (block == 6 && pos == 0) {
		if (space[space_y][space_x + 2] != 0
				|| space[space_y + 1][space_x + 3] != 0) {
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		} else {
			tempMove = tempMove + 30;
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 6 && pos == 1) {
		if (space[space_y][space_x + 1] != 0
				|| space[space_y + 1][space_x + 2] != 0
				|| space[space_y + 2][space_x + 1] != 0) {
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 6 && pos == 2) {
		if (space[space_y][space_x + 3] != 0
				|| space[space_y + 1][space_x + 2] != 0) {
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 6 && pos == 3) {
		if (space[space_y][space_x + 3] != 0
				|| space[space_y + 1][space_x + 3] != 0
				|| space[space_y + 2][space_x + 3] != 0) {
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		} else {
			tempMove = tempMove + 30;
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
}
void change_block(){
	//randBlock=6;
	nextBlock(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
	if (randBlock3 == 0 || randBlock3 == 7) {
		tempMove = 0;
		tempDown = 0;
		I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		pos = 0;
	} else if (randBlock3 == 1) {
		tempMove = 0;
		tempDown = 0;
		O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
	} else if (randBlock3 == 2) {
		tempMove = 0;
		tempDown = 0;
		L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		pos = 0;
	} else if (randBlock3 == 3) {
		tempMove = 0;
		tempDown = 0;
		J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		pos = 0;
	} else if (randBlock3 == 4) {
		tempMove = 0;
		tempDown = 0;
		S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		pos = 0;
	} else if (randBlock3 == 5) {
		tempMove = 0;
		tempDown = 0;
		Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		pos = 0;
	} else if (randBlock3 == 6) {
		tempMove = 0;
		tempDown = 0;
		T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		pos = 0;
	}
	newBlock = 0;
}
void drop(int block){
	array(tempMove, tempDown);
	//blok I
	if ((block == 0 || block == 7) && pos == 0) {
		if (space[space_y+4][space_x]!=0){
			newBlock = 1;
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			array_I_h();
			ryder();
		}else{
			tempDown = tempDown + 30;
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if ((block == 0 || block == 7) && pos == 1) {
		if (space[space_y+1][space_x]!=0 || space[space_y+1][space_x+1]!=0|| space[space_y+1][space_x+2]!=0|| space[space_y+1][space_x+3]!=0){
			newBlock = 1;
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			array_I_v();
			ryder();
		}else{
			tempDown = tempDown + 30;
			I_block_v(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//Blok O
	if (block == 1) {
		if (space[space_y+2][space_x]!=0 || space[space_y+2][space_x+1]!=0){
			newBlock = 1;
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
			array_O();
			ryder();
		}else{
			tempDown = tempDown + 30;
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	}
	//Blok L
	if (block == 2 && pos == 0) {
		if (space[space_y+2][space_x+2]!=0 || space[space_y+2][space_x+1]!=0|| space[space_y+2][space_x]!=0){
				newBlock = 1;
				L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
				array_L_1();
				ryder();
		} else{
			tempDown = tempDown + 30;
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if (block == 2 && pos == 1) {
		if (space[space_y+3][space_x]!=0 || space[space_y+3][space_x+1]!=0){
				newBlock = 1;
				L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_L_2();
				ryder();
		} else{
			tempDown = tempDown + 30;
			L_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	} else if (block == 2 && pos == 2) {
		if (space[space_y+2][space_x]!=0 || space[space_y+1][space_x+1]!=0|| space[space_y+1][space_x+2]!=0){
				newBlock = 1;
				L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_L_3();
				ryder();
		} else{
			tempDown = tempDown + 30;
			L_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	} else if (block == 2 && pos == 3) {
		if (space[space_y+1][space_x]!=0 || space[space_y+3][space_x+1]!=0){
				newBlock = 1;
				L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_L_4();
				ryder();
		} else{
			tempDown = tempDown + 30;
			L_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//Blok J
	if (block == 3 && pos == 0) {
		if (space[space_y+2][space_x]!=0 || space[space_y+2][space_x+1]!=0|| space[space_y+2][space_x+2]!=0){
				newBlock = 1;
				J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
				array_J_1();
				ryder();
		} else{
			tempDown = tempDown + 30;
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if (block == 3 && pos == 1) {
		if (space[space_y+3][space_x]!=0 || space[space_y+1][space_x+1]!=0){
				newBlock = 1;
				J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_J_2();
				ryder();
		} else{
			tempDown = tempDown + 30;
			J_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	} else if (block == 3 && pos == 2) {
		if (space[space_y+1][space_x]!=0 || space[space_y+1][space_x+1]!=0|| space[space_y+2][space_x+2]!=0){
				newBlock = 1;
				J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_J_3();
				ryder();
		} else{
			tempDown = tempDown + 30;
			J_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	} else if (block == 3 && pos == 3) {
		if (space[space_y+3][space_x]!=0 || space[space_y+3][space_x+1]!=0){
				newBlock = 1;
				J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_J_4();
				ryder();
		} else{
			tempDown = tempDown + 30;
			J_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//Blok S
	if (block == 4 && pos == 0) {
		if (space[space_y+2][space_x]!=0 || space[space_y+2][space_x+1]!=0|| space[space_y+1][space_x+2]!=0){
				newBlock = 1;
				S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
				array_S_1();
				ryder();
		} else{
			tempDown = tempDown + 30;
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}
	} else if (block == 4 && pos == 1) {
		if (space[space_y+2][space_x]!=0 || space[space_y+3][space_x+1]!=0){
				newBlock = 1;
				S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_S_2();
				ryder();
		} else{
			tempDown = tempDown + 30;
			S_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//blok Z
	if (block == 5 && pos == 0) {
		if (space[space_y+1][space_x]!=0 || space[space_y+2][space_x+1]!=0|| space[space_y+2][space_x+2]!=0){
				newBlock = 1;
				Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
				array_Z_1();
				ryder();
		} else{
			tempDown = tempDown + 30;
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 5 && pos == 1) {
		if (space[space_y+3][space_x]!=0 || space[space_y+2][space_x+1]!=0){
				newBlock = 1;
				Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_Z_2();
				ryder();
		} else{
			tempDown = tempDown + 30;
			Z_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
	//Blok T
	if (block == 6 && pos == 0) {
		if (space[space_y+2][space_x]!=0 || space[space_y+2][space_x+1]!=0|| space[space_y+2][space_x+2]!=0){
				newBlock = 1;
				T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
				array_T_1();
				ryder();
		} else{
			tempDown = tempDown + 30;
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 0, 0);
		}

	} else if (block == 6 && pos == 1) {
		if (space[space_y+3][space_x]!=0 || space[space_y+2][space_x+1]!=0){
				newBlock = 1;
				T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_T_2();
				ryder();
		} else{
			tempDown = tempDown + 30;
			T_block_2(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 6 && pos == 2) {
		if (space[space_y+1][space_x]!=0 || space[space_y+2][space_x+1]!=0|| space[space_y+1][space_x+2]!=0){
				newBlock = 1;
				T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_T_3();
				ryder();
		} else{
			tempDown = tempDown + 30;
			T_block_3(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}

	} else if (block == 6 && pos == 3) {
		if (tempMove < -90) {
			if (space[space_y + 2][space_x] != 0
					|| space[space_y + 3][space_x + 1] != 0) {
				newBlock = 1;
				T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
				array_T_4();
				ryder();

			} else {
				tempDown = tempDown + 30;
				T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			}
		} else if (space[space_y + 2][space_x + 1] != 0
				|| space[space_y + 3][space_x + 2] != 0) {
			newBlock = 1;
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
			array_T_4();
			ryder();

		} else {
			tempDown = tempDown + 30;
			T_block_4(pFrames[dispCtrl.curFrame], DEMO_STRIDE);
		}
	}
}
void StopGame(){
	int a = 0;
	for(a = 0; a < 10; a++) {
		if(space[0][a] != 0) {
			start = 0;
			memset(space, 0, sizeof(space[0][0]) * 15 * 10);
			if(pointScore > HighScore[4]){
				HighScore[0] = HighScore[1];
				HighScore[1] = HighScore[2];
				HighScore[2] = HighScore[3];
				HighScore[3] = HighScore[4];
				HighScore[4] = pointScore;
			} else if(pointScore > HighScore[3]){
				HighScore[0] = HighScore[1];
				HighScore[1] = HighScore[2];
				HighScore[2] = HighScore[3];
				HighScore[3] = pointScore;
			} else if(pointScore > HighScore[2]){
				HighScore[0] = HighScore[1];
				HighScore[1] = HighScore[2];
				HighScore[2] = pointScore;
			} else if(pointScore > HighScore[1]){
				HighScore[0] = HighScore[1];
				HighScore[1] = pointScore;
			} else if(pointScore > HighScore[0]){
				HighScore[0] = pointScore;
			}
			Menu(pFrames[dispCtrl.curFrame], dispCtrl.vMode.width, dispCtrl.vMode.height, DEMO_STRIDE);
		}
	}
}
void ryder(){
	int i = 14;
	for (i; i > -1; i--) {
		int linje = 0;
		for (int j = 0; j < 10; j++) {
			if (space[i][j] != 0) {
				linje++;
			}
		}
		if (linje == 10) {
			update(pFrames[dispCtrl.curFrame], DEMO_STRIDE, i);
			i = 15;
		}
	}

}
void nextBlock(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	tempDown = 0;
	tempMove = 0;
	randBlock3 = randBlock2;
	randBlock = rand() % 8;
	if (randBlock == randBlock2) {
		randBlock = rand() % 8;
	}
	randBlock2 = randBlock;
	if(start == 1){
		fwidth = 120;
		fheight = 120;
		fxcoi = 500 * 3;
		fycoi = 20;
		for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

			iPixelAddr = xcoi + fycoi * stride;
			for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
				frame[iPixelAddr] = backgroundGreen;
				frame[iPixelAddr + 1] = backgroundBlue;
				frame[iPixelAddr + 2] = backgroundRed;
				iPixelAddr += stride;
			}
		}

		if(randBlock2 == 0 || randBlock2 == 7){
			I_block_h(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 280, 20);
		}else if(randBlock2 == 1){
			O_block(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 270, 40);
		}else if(randBlock2 == 2){
			L_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 260, 40);
		}else if(randBlock2 == 3){
			J_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 260, 40);
		}else if(randBlock2 == 4){
			S_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 260, 40);
		}else if(randBlock2 == 5){
			Z_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 260, 40);
		}else if(randBlock2 == 6){
			T_block_1(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 260, 40);
		}
	}

}

void array (int x, int y){
	space_y=y/30;
	switch(x){

	case -90:
		space_x=0;
		break;
	case -60:
		space_x=1;
		break;
	case -30:
		space_x=2;
		break;
	case 0:
		space_x=3;
		break;
	case 30:
		space_x=4;
		break;
	case 60:
		space_x=5;
		break;
	case 90:
		space_x=6;
		break;
	case 120:
		space_x=7;
		break;
	case 150:
		space_x=8;
		break;
	case 180:
		space_x=9;
		break;
	default:
		break;

	}
}
void array_O(){
	space[space_y][space_x]=2;
	space[space_y+1][space_x]=2;
	space[space_y][space_x+1]=2;
	space[space_y+1][space_x+1]=2;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_I_h(){
	space[space_y][space_x]=1;
	space[space_y+1][space_x]=1;
	space[space_y+2][space_x]=1;
	space[space_y+3][space_x]=1;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_I_v (){
	space[space_y][space_x]=1;
	space[space_y][space_x+1]=1;
	space[space_y][space_x+2]=1;
	space[space_y][space_x+3]=1;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}

void array_L_1(){
	space[space_y][space_x+2]=3;
	space[space_y+1][space_x]=3;
	space[space_y+1][space_x+1]=3;
	space[space_y+1][space_x+2]=3;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_L_2(){
	space[space_y][space_x]=3;
	space[space_y+1][space_x]=3;
	space[space_y+2][space_x]=3;
	space[space_y+2][space_x+1]=3;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_L_3(){
	space[space_y][space_x]=3;
	space[space_y][space_x+1]=3;
	space[space_y][space_x+2]=3;
	space[space_y+1][space_x]=3;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_L_4(){
	space[space_y][space_x]=3;
	space[space_y][space_x+1]=3;
	space[space_y+1][space_x+1]=3;
	space[space_y+2][space_x+1]=3;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}

void array_J_1(){
	space[space_y][space_x]=4;
	space[space_y+1][space_x]=4;
	space[space_y+1][space_x+1]=4;
	space[space_y+1][space_x+2]=4;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_J_2(){
	space[space_y][space_x]=4;
	space[space_y][space_x+1]=4;
	space[space_y+1][space_x]=4;
	space[space_y+2][space_x]=4;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_J_3(){
	space[space_y][space_x]=4;
	space[space_y][space_x+1]=4;
	space[space_y][space_x+2]=4;
	space[space_y+1][space_x+2]=4;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_J_4(){
	space[space_y][space_x+1]=4;
	space[space_y+1][space_x+1]=4;
	space[space_y+2][space_x+1]=4;
	space[space_y+2][space_x]=4;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}

void array_S_1(){
	space[space_y][space_x+1]=5;
	space[space_y][space_x+2]=5;
	space[space_y+1][space_x+1]=5;
	space[space_y+1][space_x]=5;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_S_2(){
	space[space_y][space_x]=5;
	space[space_y+1][space_x]=5;
	space[space_y+1][space_x+1]=5;
	space[space_y+2][space_x+1]=5;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}

void array_Z_1(){
	space[space_y][space_x]=6;
	space[space_y][space_x+1]=6;
	space[space_y+1][space_x+1]=6;
	space[space_y+1][space_x+2]=6;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_Z_2(){
	space[space_y][space_x+1]=6;
	space[space_y+1][space_x+1]=6;
	space[space_y+1][space_x]=6;
	space[space_y+2][space_x]=6;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}

void array_T_1(){
	space[space_y][space_x+1]=7;
	space[space_y+1][space_x]=7;
	space[space_y+1][space_x+1]=7;
	space[space_y+1][space_x+2]=7;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_T_2(){
	space[space_y][space_x]=7;
	space[space_y+1][space_x]=7;
	space[space_y+1][space_x+1]=7;
	space[space_y+2][space_x]=7;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_T_3(){
	space[space_y][space_x]=7;
	space[space_y][space_x+1]=7;
	space[space_y][space_x+2]=7;
	space[space_y+1][space_x+1]=7;
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}
void array_T_4(){
	if(tempMove < -90){
		space[space_y][space_x + 1] = 7;
		space[space_y + 1][space_x + 1] = 7;
		space[space_y + 1][space_x] = 7;
		space[space_y + 2][space_x + 1] = 7;
	}else{
		space[space_y][space_x + 2] = 7;
		space[space_y + 1][space_x + 2] = 7;
		space[space_y + 1][space_x + 1] = 7;
		space[space_y + 2][space_x + 2] = 7;
	}
	if (dropcheck==1){
	XTmrCtr_SetResetValue(&TMRInst, 0, TMR_LOAD1);
	dropcheck=0;
	}
}

void Background(u8 *frame, u32 width, u32 height, u32 stride){

	u32 xcoi, ycoi;
	u32 iPixelAddr;
	for(xcoi = 0; xcoi < (width*3); xcoi+=3){
		iPixelAddr = xcoi;

		for (ycoi = 0; ycoi < height; ycoi++) {
			frame[iPixelAddr] = backgroundGreen;
			frame[iPixelAddr + 1] = backgroundBlue;
			frame[iPixelAddr + 2] = backgroundRed;
			iPixelAddr += stride;
		}
	}

	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);

}
void gameField(){
	//Bottom line
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 300, 10, (170 * 3), 450, 0, 0, 0);
	//Left side
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 10, 460, (160 * 3), 0, 0, 0, 0);
	//Right side
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 10, 460, (470 * 3), 0, 0, 0, 0);
	//Infobox
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 140, 140, (10 * 3), 10, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 120, 120, (20 * 3), 20, 128, 128, 128);
	//next block
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 140, 140, (490 * 3), 10, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 120, 120, (500 * 3), 20, 128, 128, 128);

	score();
	numbers(pointScore, 0);
	numbers(HighScore[4], 40);
	numbers(HighScore[3], 51);
	numbers(HighScore[2], 62);
	numbers(HighScore[1], 73);
	numbers(HighScore[0], 84);
	level();
	highScore();
	levelNumber(levelNum);
}
void clear(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;

	fwidth = cwidth;
	fheight = cheight;
	fxcoi = cxcoi;
	fycoi = cycoi;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = backgroundGreen;
			frame[iPixelAddr + 1] = backgroundBlue;
			frame[iPixelAddr + 2] = backgroundRed;
			iPixelAddr += stride;
		}
	}

	fwidth = cwidth2;
	fheight = cheight2;
	fxcoi = cxcoi2;
	fycoi = cycoi2;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = backgroundGreen;
			frame[iPixelAddr + 1] = backgroundBlue;
			frame[iPixelAddr + 2] = backgroundRed;
			iPixelAddr += stride;
		}
	}


}
void update(u8 *frame, u32 stride, int k){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	int b = 0;
	//Update array
	int a = k;
	for (k; k > -1; k--) {
		for (int z = 0; z < 10; z++) {
			space[k][z] = space[k - 1][z];
		}
	}
	for (int z = 0; z < 10; z++) {
		space[0][z] = 0;
	}
	//Print updatede array
	for (a; a > -1; a--) {
		for (b = 0; b < 10; b++) {

			if (space[a][b] == 1) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 0.0;
				fBlue = 255.0;
				fGreen = 255.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}

				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}
				}
			} else if (space[a][b] == 2) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 255.0;
				fBlue = 0.0;
				fGreen = 255.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}
				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}
				}
			} else if (space[a][b] == 3) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 255.0;
				fBlue = 0.0;
				fGreen = 165.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}
				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}
				}
			} else if (space[a][b] == 4) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 0.0;
				fBlue = 255.0;
				fGreen = 0.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}
				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}
				}
			} else if (space[a][b] == 5) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 0.0;
				fBlue = 0.0;
				fGreen = 255.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}
				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}

				}
			} else if (space[a][b] == 6) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 255.0;
				fBlue = 0.0;
				fGreen = 0.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}
				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}

				}
			} else if (space[a][b] == 7) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fRed = 255.0;
				fBlue = 255.0;
				fGreen = 0.0;
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = sGreen;
						frame[iPixelAddr + 1] = sBlue;
						frame[iPixelAddr + 2] = sRed;
						iPixelAddr += stride;
					}
				}
				fxcoi = (171 + (b * 30)) * 3;
				fycoi = 1 + (a * 30);
				fwidth = 28;
				fheight = 28;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = fGreen;
						frame[iPixelAddr + 1] = fBlue;
						frame[iPixelAddr + 2] = fRed;
						iPixelAddr += stride;
					}

				}
			} else if (space[a][b] == 0) {
				fxcoi = (170 + (b * 30)) * 3;
				fycoi = 0 + (a * 30);
				fwidth = 30;
				fheight = 30;
				for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

					iPixelAddr = xcoi + fycoi * stride;
					for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
						frame[iPixelAddr] = backgroundGreen;
						frame[iPixelAddr + 1] = backgroundBlue;
						frame[iPixelAddr + 2] = backgroundRed;
						iPixelAddr += stride;
					}
				}
			}
		}

	}
	pointScore = 100 + pointScore;
	numbers(pointScore, 0);
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void score(){
	//S
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (30*3), 30, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (32*3), 31, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (30*3), 36, 128, 128, 128);
	//C
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (36*3), 30, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 6, (38*3), 32, 128, 128, 128);
	//O
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (42*3), 30, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 8, (44*3), 31, 128, 128, 128);
	//R
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (48*3), 30, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 3, (49*3), 31, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 4, 5, (49*3), 35, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (49*3), 35, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (50*3), 36, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (51*3), 37, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 2, (52*3), 38, 0, 0, 0);
	//E
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (54*3), 30, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (56*3), 32, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (56*3), 36, 128, 128, 128);
	//Semi Colon
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (61*3), 32, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (61*3), 36, 0, 0, 0);
}
void numbers(int score, int high){

	int i, n, d, c, a[5], b[5];
	n = 5;
	for (i = 0; i < 5; ++i, score /= 10) {
		a[i] = score % 10;
	}
	//flip array
	for (c = n - 1, d = 0; c >= 0; c--, d++)
		b[d] = a[c];
	for (c = 0; c < n; c++) {
		a[c] = b[c];
	}
	int x;
	x = sizeof(a) / sizeof(int);

	//Clear numbers
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 25, 10, (110*3), (31 + high), 128, 128, 128);
	for(i = 0; i < x; i++){
		switch(a[i]){
		case 0:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 7, ((110 + (5*i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 7, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			break;
		case 1:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 9, ((110 + 1 + (5 * i))*3), (31 + high), 0, 0, 0);
			break;
		case 2:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((110 + (5 * i))*3), (36 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((114 + (5 * i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (36 + high), 128, 128, 128);
			break;
		case 3:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((114 + (5 * i))*3), (36 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((114 + (5 * i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (36 + high), 128, 128, 128);
			break;
		case 4:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 5, ((110 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 9, ((114 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 4, ((111 + (5 * i))*3), (31 + high), 128, 128, 128);
			break;
		case 5:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((110 + (5 * i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((114 + (5 * i))*3), (36 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (36 + high), 128, 128, 128);
			break;
		case 6:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((110 + (5 * i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 3, ((110 + (5 * i))*3), (36 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (36 + high), 128, 128, 128);
			break;
		case 7:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 4, 1, ((110 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 9, ((114 + (5 * i))*3), (31 + high), 0, 0, 0);
			break;
		case 8:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 7, ((110 + (5 * i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 7, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 1, ((111 + (5 * i))*3), (35 + high), 0, 0, 0);
			break;
		case 9:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((111 + (5 * i))*3), (31 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((114 + (5 * i))*3), (36 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 3, ((110 + (5 * i))*3), (32 + high), 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (32 + high), 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((111 + (5 * i))*3), (36 + high), 128, 128, 128);
			break;
		}
	}

}
void level(){
	//L
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (30*3), 50, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 8, (32*3), 50, 128, 128, 128);
	//E
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (36*3), 50, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (38*3), 52, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (38*3), 56, 128, 128, 128);
	//V
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (42*3), 50, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 7, (42*3), 53, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 5, (43*3), 56, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 5, (45*3), 56, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 7, (46*3), 53, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 2, (43*3), 50, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 5, (44*3), 50, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 2, (45*3), 50, 128, 128, 128);
	//E
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (48*3), 50, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (50*3), 52, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (50*3), 56, 128, 128, 128);
	//L
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (54*3), 50, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 8, (56*3), 50, 128, 128, 128);
	//Semi colon
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (61*3), 52, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (61*3), 56, 0, 0, 0);
}
void levelNumber(int level){
	int i, n, d, c, a[2], b[2];
	n = 2;
	for (i = 0; i < 2; ++i, level /= 10) {
		a[i] = level % 10;
	}
	//flip array
	for (c = n - 1, d = 0; c >= 0; c--, d++)
		b[d] = a[c];
	for (c = 0; c < n; c++) {
		a[c] = b[c];
	}
	int x;
	x = sizeof(a) / sizeof(int);
	//Clear numbers
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 25, 10, (110*3), 51, 128, 128, 128);

	for (i = 0; i < x; i++) {
		switch (a[i]) {
		case 0:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 7, ((125 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 7, ((126 + (5 * i))*3), 52, 128, 128, 128);
			break;
		case 1:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 9, ((125 + 1 +(5 * i))*3), 51, 0, 0, 0);
			break;
		case 2:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((125 + (5 * i))*3), 56, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((129 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 52, 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 56, 128, 128, 128);
			break;
		case 3:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((129 + (5 * i))*3), 56, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((129 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 52, 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 56, 128, 128, 128);
			break;
		case 4:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 5, ((125 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 9, ((129 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 4, ((126 + (5 * i))*3), 51, 128, 128, 128);
			break;
		case 5:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((125 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((129 + (5 * i))*3), 56, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 52, 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 56, 128, 128, 128);
			break;
		case 6:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((125 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 3, ((125 + (5 * i))*3), 56, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 52, 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 56, 128, 128, 128);
			break;
		case 7:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 4, 1, ((125 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 9, ((129 + (5 * i))*3), 51, 0, 0, 0);
			break;
		case 8:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 7, ((125 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 7, ((126 + (5 * i))*3), 52, 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 1, ((126 + (5 * i))*3), 55, 0, 0, 0);
			break;
		case 9:
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 9, ((126 + (5 * i))*3), 51, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 3, ((129 + (5 * i))*3), 56, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 3, ((125 + (5 * i))*3), 52, 0, 0, 0);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 52, 128, 128, 128);
			Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, ((126 + (5 * i))*3), 56, 128, 128, 128);
			break;
		}
	}

}
void highScore(){
	//H
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (30*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (31*3), 70, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 4, (31*3), 76, 128, 128, 128);
	//I
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (36*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 8, (36*3), 71, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 8, (39*3), 71, 128, 128, 128);
	//G
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 10, (43*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 8, (42*3), 71, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 8, (43*3), 71, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 2, (46*3), 72, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (44*3), 74, 0, 0, 0);
	//H
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (48*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (49*3), 70, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 4, (49*3), 76, 128, 128, 128);
	//S
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (54*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (56*3), 71, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (54*3), 76, 128, 128, 128);
	//C
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (60*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 6, (62*3), 72, 128, 128, 128);
	//O
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (66*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 8, (68*3), 71, 128, 128, 128);
	//R
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (72*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 3, (73*3), 71, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 4, 5, (73*3), 75, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (73*3), 75, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (74*3), 76, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 2, 2, (75*3), 77, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 2, (76*3), 78, 0, 0, 0);
	//E
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 5, 10, (78*3), 70, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (80*3), 72, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 2, (80*3), 76, 128, 128, 128);
	//Semi colon
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (85*3), 72, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 3, 3, (85*3), 76, 0, 0, 0);

}
void Menu(u8 *frame, u32 width, u32 height, u32 stride){
	u32 iPixelAddr;
	u32 xcoi, ycoi;
	for (xcoi = 0; xcoi < (width * 3); xcoi += 3) {
		iPixelAddr = xcoi;

		for (ycoi = 0; ycoi < height; ycoi++) {
			frame[iPixelAddr] = backgroundGreen;
			frame[iPixelAddr + 1] = backgroundBlue;
			frame[iPixelAddr + 2] = backgroundRed;
			iPixelAddr += stride;
		}
	}
	//T
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 100, 20, (50 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 120, (90*3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 98, 18, (51*3), 181, 255, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 18, 118, (91*3), 181, 255, 0, 0);
	//E
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 80, 120, (160 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 78, 118, (161 * 3), 181, 255, 165, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 60, 20, (180 * 3), 200, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 60, 18, (181 * 3), 201, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 60, 40, (180 * 3), 240, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 60, 38, (181 * 3), 241, 128, 128, 128);
	//T
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 100, 20, (250 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 120, (290 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 98, 18, (251 * 3), 181, 255, 255, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 18, 118, (291 * 3), 181, 255, 255, 0);
	//R
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 120, (360 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 19, 118, (361 * 3), 181, 0, 255, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 80, (380 * 3), 180, 0, 255, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 40, (380 * 3), 200, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 39, 38, (381 * 3), 201, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 40, (420 * 3), 200, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 40, (420 * 3), 260, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 19, 38, (420 * 3), 201, 0, 255, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 18, 38, (421 * 3), 261, 0, 255, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 1, (380 * 3), 260, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 80, (420 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 1, (380 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 40, (380 * 3), 260, 0, 0, 0);
	//I
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 120, (450 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 18, 118, (451 * 3), 181, 0, 255, 255);
	//S
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 20, (500 * 3), 180, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 20, (500 * 3), 280, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 38, 18, (501 * 3), 181, 255, 0, 255);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 38, 18, (501 * 3), 281, 255, 0, 255);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 80, 80, (480 * 3), 200, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 78, 78, (481 * 3), 201, 255, 0, 255);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 80, 1, (480 * 3), 219, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 80, 1, (480 * 3), 260, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 80, (499 * 3), 200, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 1, 80, (540 * 3), 200, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 40, 80, (500 * 3), 200, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 80, 40, (480 * 3), 220, 128, 128, 128);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 20, (500 * 3), 220, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 20, 20, (520 * 3), 240, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 18, 18, (501 * 3), 221, 255, 0, 255);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 18, 18, (521 * 3), 241, 255, 0, 255);

	//info box
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 140, 140, (10*3), 10, 0, 0, 0);
	Draw(pFrames[dispCtrl.curFrame], DEMO_STRIDE, 120, 120, (20*3), 20, 128, 128, 128);
	//stats from last game
	score();
	numbers(pointScore, 0);
	numbers(HighScore[4], 40);
	numbers(HighScore[3], 51);
	numbers(HighScore[2], 62);
	numbers(HighScore[1], 73);
	numbers(HighScore[0], 84);
	level();
	highScore();
	levelNumber(levelNum);

	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
//Blocks
void I_block_h(u8 *frame, u32 stride, int nextX, int nextY){

	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	//draw block
	fRed = 0.0;
	fBlue = 255.0;
	fGreen = 255.0;
	fwidth = 30;
	fheight = 120;

	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 180) {
		tempMove = 180;
	}
	fxcoi = (260 + tempMove + nextX)* 3;
	if(tempDown > 330){
		tempDown = 330;
		newBlock = 1;
		array(tempMove, tempDown);
		array_I_h();
		ryder();
	}
	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	cxcoi2 = 0;
	cycoi2 = 0;
	cwidth2 = 0;
	cheight2 = 0;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 118;
	fxcoi = (261 + tempMove + nextX)* 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 60 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 90 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void I_block_v(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	//Draw block
	fRed = 0.0;
	fBlue = 255.0;
	fGreen = 255.0;

	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 90) {
		tempMove = 90;
	}
	fwidth = 120;
	fheight = 30;
	fxcoi = (260 + tempMove)* 3;

	if(tempDown > 420){
		tempDown = 420;
		newBlock = 1;
		array(tempMove, tempDown);
		array_I_v();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	cxcoi2 = 0;
	cycoi2 = 0;
	cwidth2 = 0;
	cheight2 = 0;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 118;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (350 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void O_block(u8 *frame, u32 stride, int nextX, int nextY){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	//Draw block
	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 255.0;

	fwidth = 60;
	fheight = 60;
	if (tempMove < -90) {
		tempMove = -90;
	}
	if (tempMove >= 150) {
		tempMove = 150;
	}
	fxcoi = (260 + tempMove + nextX) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_O();
		ryder();
	}
	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	cxcoi2 = 0;
	cycoi2 = 0;
	cwidth2 = 0;
	cheight2 = 0;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 58;
	fxcoi = (261 + tempMove + nextX) * 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 60;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 0 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 60;
	fheight = 1;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void L_block_1(u8 *frame, u32 stride, int nextX, int nextY){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;

	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 165.0;
	//draw block
	if(tempMove < -90){
		tempMove = -90;
	} else if (tempMove > 120){
		tempMove = 120;
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (320 + tempMove + nextX) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_L_1();
		ryder();
	}

	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 90;
	fheight = 30;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (321 + tempMove + nextX) * 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 88;
	fheight = 28;
	fxcoi = (261 + tempMove + nextX) * 3;
	fycoi = 31 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void L_block_2(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 165.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 30;
	fheight = 90;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_L_2();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 60 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 88;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 61 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 30 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void L_block_3(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 165.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 90;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_L_3();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 88;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void L_block_4(u8 *frame, u32 stride){

	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 165.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_L_4();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void J_block_1(u8 *frame, u32 stride, int nextX, int nextY){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 0.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (260 + tempMove + nextX) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_J_1();
		ryder();
	}
	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (261 + tempMove + nextX) * 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (291 + tempMove + nextX) * 3;
	fycoi = 31 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void J_block_2(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 0.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_J_2();
		ryder();
	}

	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

			iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void J_block_3(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 0.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 90;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_J_3();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 88;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (321 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void J_block_4(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 0.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 30;
	fheight = 90;
	fxcoi = (290 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_J_4();
		ryder();

	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 60 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 88;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 61 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 30 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void S_block_1(u8 *frame, u32 stride, int nextX, int nextY){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 0.0;
	fBlue = 0.0;
	fGreen = 255.0;

	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_S_1();
		ryder();
	}
	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (291 + tempMove + nextX) * 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (261 + tempMove + nextX) * 3;
	fycoi = 31 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove + nextX) * 3;
	fycoi = 0 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void S_block_2(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 0.0;
	fBlue = 0.0;
	fGreen = 255.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_S_2();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 30 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void Z_block_1(u8 *frame, u32 stride, int nextX, int nextY){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (260 + tempMove + nextX) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_Z_1();
		ryder();
	}
	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 60;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (261 + tempMove + nextX) * 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 58;
	fheight = 28;
	fxcoi = (291 + tempMove + nextX) * 3;
	fycoi = 31 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 0 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void Z_block_2(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 0.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (290 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_Z_2();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 60;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 58;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 30 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void T_block_1(u8 *frame, u32 stride, int nextX, int nextY){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_T_1();
		ryder();
	}
	fycoi = 0 + tempDown + nextY;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 90;
	fheight = 30;
	fxcoi = (260 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (291 + tempMove + nextX) * 3;
	fycoi = 1 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 88;
	fheight = 28;
	fxcoi = (261 + tempMove + nextX) * 3;
	fycoi = 31 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove + nextX) * 3;
	fycoi = 30 + tempDown + nextY;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void T_block_2(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 150) {
		tempMove = 150;
	}
	fwidth = 30;
	fheight = 90;
	fxcoi = (260 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_T_2();
		ryder();

	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 88;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 30 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (260 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void T_block_3(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -90) {
		tempMove = -90;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 90;
	fheight = 30;
	fxcoi = (260 + tempMove) * 3;

	if(tempDown > 390){
		tempDown = 390;
		newBlock = 1;
		array(tempMove, tempDown);
		array_T_3();
		ryder();
	}
	fycoi = 0 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 30 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 88;
	fheight = 28;
	fxcoi = (261 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 1;
	fheight = 30;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 0 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
void T_block_4(u8 *frame, u32 stride){
	u32 iPixelAddr;
	u32 fwidth, fheight;
	u32 xcoi, ycoi;
	u32 fxcoi, fycoi;
	double fRed, fBlue, fGreen;
	fRed = 255.0;
	fBlue = 255.0;
	fGreen = 0.0;
	//draw block
	if (tempMove < -120) {
		tempMove = -120;
	} else if (tempMove > 120) {
		tempMove = 120;
	}
	fwidth = 30;
	fheight = 30;
	fxcoi = (290 + tempMove) * 3;
	if(tempDown > 360){
		tempDown = 360;
		newBlock = 1;
		array(tempMove, tempDown);
		array_T_4();
		ryder();
	}
	fycoi = 30 + tempDown;

	cxcoi = fxcoi;
	cycoi = fycoi;
	cwidth = fwidth;
	cheight = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 90;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 0 + tempDown;

	cxcoi2 = fxcoi;
	cycoi2 = fycoi;
	cwidth2 = fwidth;
	cheight2 = fheight;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 28;
	fxcoi = (291 + tempMove) * 3;
	fycoi = 31 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 28;
	fheight = 88;
	fxcoi = (321 + tempMove) * 3;
	fycoi = 1 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 30 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	fwidth = 30;
	fheight = 1;
	fxcoi = (320 + tempMove) * 3;
	fycoi = 60 + tempDown;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = sGreen;
			frame[iPixelAddr + 1] = sBlue;
			frame[iPixelAddr + 2] = sRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
//Draw function for non-block graphics
void Draw(u8 *frame, u32 stride, u32 fwidth, u32 fheight, u32 fxcoi, u32 fycoi, double fRed, double fGreen, double fBlue){
	u32 iPixelAddr;
	u32 xcoi, ycoi;
	for (xcoi = fxcoi; xcoi < (fxcoi + fwidth * 3); xcoi += 3) {

		iPixelAddr = xcoi + fycoi * stride;
		for (ycoi = fycoi; ycoi < (fycoi + fheight); ycoi++) {
			frame[iPixelAddr] = fGreen;
			frame[iPixelAddr + 1] = fBlue;
			frame[iPixelAddr + 2] = fRed;
			iPixelAddr += stride;
		}
	}
	Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
}
