
#ifndef __HARDWAREPROFILE_HEXLIGHT_H__
#define __HARDWAREPROFILE_HEXLIGHT_H__



 	/*******************************************************************/
    /******** USB stack hardware selection options *********************/
    /*******************************************************************/
    //This section is the set of definitions required by the MCHPFSUSB
    //  framework.  These definitions tell the firmware what mode it is
    //  running in, and where it can find the results to some information
    //  that the stack needs.
    //These definitions are required by every application developed with
    //  this revision of the MCHPFSUSB framework.  Please review each
    //  option carefully and determine which options are desired/required
    //  for your application.

    //#define USE_SELF_POWER_SENSE_IO
    #define tris_self_power        
    #define self_power          1

    //#define USE_USB_BUS_SENSE_IO
    #define tris_usb_bus_sense      
    #define USB_BUS_SENSE       1 

    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Application specific definitions *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/

    /** Board definition ***********************************************/
    //These defintions will tell the main() function which board is
    //  currently selected.  This will allow the application to add
    //  the correct configuration bits as well use the correct
    //  initialization functions for the board.  These defitions are only
    //  required for this demo.  They are not required in
    //  final application design.



    // STATUS LEDs
	#define mLED              LATBbits.LATB7
	
	#define BlinkLED() (mLED = ((ReadCoreTimer() & 0x0800000) != 0))
	#define InitLED() do{	\
							TRISBbits.TRISB7 = 0;  \
							TRISBbits.TRISB8 = 0;  \
							LATBbits.LATB7 = 0; LATBbits.LATB8 = 0;\
						}while(0)
	
	// SW1
	#define ReadSwitchStatus() (PORTReadBits(IOPORT_A, BIT_4) & BIT_4)

   // Error indication	
	#define Error()   do{ LATBbits.LATB7 = 0; LATBbits.LATB8 = 1; } while(0);

	

#endif
