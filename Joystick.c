/*
Nintendo Switch Fightstick - Proof-of-Concept

Based on the LUFA library's Low-Level Joystick Demo
	(C) Dean Camera
Based on the HORI's Pokken Tournament Pro Pad design
	(C) HORI

This project implements a modified version of HORI's Pokken Tournament Pro Pad
USB descriptors to allow for the creation of custom controllers for the
Nintendo Switch. This also works to a limited degree on the PS3.

Since System Update v3.0.0, the Nintendo Switch recognizes the Pokken
Tournament Pro Pad as a Pro Controller. Physical design limitations prevent
the Pokken Controller from functioning at the same level as the Pro
Controller. However, by default most of the descriptors are there, with the
exception of Home and Capture. Descriptor modification allows us to unlock
these buttons for our use.
*/

/** \file
 *
 *  Main source file for the posts printer demo. This file contains the main tasks of
 *  the demo and is responsible for the initial application hardware configuration.
 */

#include "Joystick.h"


#define CPU_PRESCALE(n) (CLKPR = 0x80, CLKPR = (n))
#define BAUD_RATE 57600


extern const uint8_t image_data[0x12c1] PROGMEM;

// Main entry point.
int main(void) {
	//CPU_PRESCALE(0);
	
	// We'll start by performing hardware and peripheral setup.
	SetupHardware();
	// We'll then enable global interrupts for our use.
	GlobalInterruptEnable();
	// Once that's done, we'll enter an infinite loop.
	uart_init(BAUD_RATE);
	for (;;)
	{
		// We need to run our task to process and deliver data for our IN and OUT endpoints.
		HID_Task();
		// We also need to run the main USB management task.
		USB_USBTask();
	}
}

// Configures hardware and peripherals, such as the USB peripherals.
void SetupHardware(void) {
	// We need to disable watchdog if enabled by bootloader/fuses.
	MCUSR &= ~(1 << WDRF);
	wdt_disable();

	// We need to disable clock division before initializing the USB hardware.
	clock_prescale_set(clock_div_1);
	// We can then initialize our hardware and peripherals, including the USB stack.

	#ifdef ALERT_WHEN_DONE
	// Both PORTD and PORTB will be used for the optional LED flashing and buzzer.
	#warning LED and Buzzer functionality enabled. All pins on both PORTB and \
PORTD will toggle when printing is done.
	DDRD  = 0xFF; //Teensy uses PORTD
	PORTD =  0x0;
                  //We'll just flash all pins on both ports since the UNO R3
	DDRB  = 0xFF; //uses PORTB. Micro can use either or, but both give us 2 LEDs
	PORTB =  0x0; //The ATmega328P on the UNO will be resetting, so unplug it?
	#endif
	// The USB stack should be initialized last.
	USB_Init();
}

// Fired to indicate that the device is enumerating.
void EVENT_USB_Device_Connect(void) {
	// We can indicate that we're enumerating here (via status LEDs, sound, etc.).
}

// Fired to indicate that the device is no longer connected to a host.
void EVENT_USB_Device_Disconnect(void) {
	// We can indicate that our device is not ready (via status LEDs, sound, etc.).
}

// Fired when the host set the current configuration of the USB device after enumeration.
void EVENT_USB_Device_ConfigurationChanged(void) {
	bool ConfigSuccess = true;

	// We setup the HID report endpoints.
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
	ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);

	// We can read ConfigSuccess to indicate a success or failure at this point.
}

// Process control requests sent to the device from the USB host.
void EVENT_USB_Device_ControlRequest(void) {
	// We can handle two control requests: a GetReport and a SetReport.

	// Not used here, it looks like we don't receive control request from the Switch.
}

// Process and deliver data from IN and OUT endpoints.
void HID_Task(void) {
	// If the device isn't connected and properly configured, we can't do anything here.
	if (USB_DeviceState != DEVICE_STATE_Configured)
		return;

	// We'll start with the OUT endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);
	// We'll check to see if we received something on the OUT endpoint.
	if (Endpoint_IsOUTReceived())
	{
		// If we did, and the packet has data, we'll react to it.
		if (Endpoint_IsReadWriteAllowed())
		{
			// We'll create a place to store our data received from the host.
			USB_JoystickReport_Output_t JoystickOutputData;
			// We'll then take in that data, setting it up in our storage.
			while(Endpoint_Read_Stream_LE(&JoystickOutputData, sizeof(JoystickOutputData), NULL) != ENDPOINT_RWSTREAM_NoError);
			// At this point, we can react to this data.

			// However, since we're not doing anything with this data, we abandon it.
		}
		// Regardless of whether we reacted to the data, we acknowledge an OUT packet on this endpoint.
		Endpoint_ClearOUT();
	}

	// We'll then move on to the IN endpoint.
	Endpoint_SelectEndpoint(JOYSTICK_IN_EPADDR);
	// We first check to see if the host is ready to accept data.
	if (Endpoint_IsINReady())
	{
		// We'll create an empty report.
		USB_JoystickReport_Input_t JoystickInputData;
		// We'll then populate this report with what we want to send to the host.
		GetNextReport(&JoystickInputData);
		// Once populated, we can output this data to the host. We do this by first writing the data to the control stream.
		while(Endpoint_Write_Stream_LE(&JoystickInputData, sizeof(JoystickInputData), NULL) != ENDPOINT_RWSTREAM_NoError);
		// We then send an IN packet on this endpoint.
		Endpoint_ClearIN();
	}
}

typedef enum {
	SYNC_CONTROLLER,
	SYNC_POSITION,
	WAITING,
	HODL,
	WAITUART,
	CONNECT
} State_t;
State_t state = SYNC_CONTROLLER;

#define ECHOES 2
int echoes = 0;
USB_JoystickReport_Input_t last_report;
char c;
int report_count = 0;
int xpos = 0;
int ypos = 0;
int portsval = 0;
int xp = 0;
int yp = 0;
int rxp = 0;
int ryp = 0;

// Prepare the next report for the host.
void GetNextReport(USB_JoystickReport_Input_t* const ReportData) {

	// Prepare an empty report
	memset(ReportData, 0, sizeof(USB_JoystickReport_Input_t));

	// We need to set our controller inputs to be centered as 128 is centered, 0 is negative movement and 255 is positive movement. 0-255 on controller inputs.
	ReportData->LX = STICK_CENTER;
	ReportData->LY = STICK_CENTER;
	ReportData->RX = STICK_CENTER;
	ReportData->RY = STICK_CENTER;
	ReportData->HAT = HAT_CENTER;

	// States and moves management
	switch (state)
	{
		case SYNC_CONTROLLER:
		// Switch Controller Authentication, Press L and R then A.
			if (report_count > 100)
			{
				report_count = 0;
				state = WAITUART;
			}
			else if (report_count == 25 || report_count == 50)
			{
				ReportData->Button |= SWITCH_L | SWITCH_R;
			}
			else if (report_count == 75 || report_count == 100)
			{
				ReportData->Button |= SWITCH_A;
			}
			report_count++;
			break;
		case CONNECT:
		// Controller Timeout
			ReportData->Button |= SWITCH_L;
			ReportData->Button |= SWITCH_R;
			_delay_ms(500);
			state = WAITUART;
			break;
		case WAITING:
			state = WAITUART;
			break;
		case HODL:
			_delay_ms(5);
			state=WAITUART;
			break;
		case WAITUART:
			 xp = uart_getchar();
			 yp = uart_getchar();
			 rxp = uart_getchar();
			 ryp = uart_getchar();
			 c = uart_getchar();

			 uart_putchar(xp);
			 uart_putchar(yp);
			 uart_putchar(rxp);
			 uart_putchar(ryp);
			 uart_putchar(c);
			 
			 if ((c & 0b10000000) == 0b10000000)
				 ReportData->Button |= SWITCH_B;
			 if ((c & 0b01000000) == 0b01000000)
				 ReportData->Button |= SWITCH_Y;
			 if ((c & 0b00100000) == 0b00100000)
				 ReportData->Button |= SWITCH_ZL;
			 if ((c & 0b00010000) == 0b00010000)
				 ReportData->Button |= SWITCH_A;
			 if ((c & 0b00001000) == 0b00001000)
				 ReportData->Button |= SWITCH_L;
			 if ((c & 0b00000100) == 0b00000100)
				 ReportData->Button |= SWITCH_MINUS;
			 if ((c & 0b00000010) == 0b00000010)
				 ReportData->Button |= SWITCH_CAPTURE;
			
			ReportData-> LY = yp;
			ReportData-> LX = xp;
			ReportData-> RY = ryp;
			ReportData-> RX = rxp;
			break;
	}

}
