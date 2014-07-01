/********************************************************************
 FileName:     	usb_descriptors.c
 Dependencies:	See INCLUDES section
 Processor:		PIC18 or PIC24 USB Microcontrollers
 Hardware:		The code is natively intended to be used on the following
 				hardware platforms: PICDEM™ FS USB Demo Board, 
 				PIC18F87J50 FS USB Plug-In Module, or
 				Explorer 16 + PIC24 USB PIM.  The firmware may be
 				modified for use on other USB platforms by editing the
 				HardwareProfile.h file.
 Complier:  	Microchip C18 (for PIC18) or C30 (for PIC24)
 Company:		Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the “Company”) for its PIC® Microcontroller is intended and
 supplied to you, the Company’s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN “AS IS” CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

*********************************************************************
-usb_descriptors.c-
-------------------------------------------------------------------
Filling in the descriptor values in the usb_descriptors.c file:
-------------------------------------------------------------------

[Device Descriptors]
The device descriptor is defined as a USB_DEVICE_DESCRIPTOR type.  
This type is defined in usb_ch9.h  Each entry into this structure
needs to be the correct length for the data type of the entry.

[Configuration Descriptors]
The configuration descriptor was changed in v2.x from a structure
to a BYTE array.  Given that the configuration is now a byte array
each byte of multi-byte fields must be listed individually.  This
means that for fields like the total size of the configuration where
the field is a 16-bit value "64,0," is the correct entry for a
configuration that is only 64 bytes long and not "64," which is one
too few bytes.

The configuration attribute must always have the _DEFAULT
definition at the minimum. Additional options can be ORed
to the _DEFAULT attribute. Available options are _SELF and _RWU.
These definitions are defined in the usb_device.h file. The
_SELF tells the USB host that this device is self-powered. The
_RWU tells the USB host that this device supports Remote Wakeup.

[Endpoint Descriptors]
Like the configuration descriptor, the endpoint descriptors were 
changed in v2.x of the stack from a structure to a BYTE array.  As
endpoint descriptors also has a field that are multi-byte entities,
please be sure to specify both bytes of the field.  For example, for
the endpoint size an endpoint that is 64 bytes needs to have the size
defined as "64,0," instead of "64,"

Take the following example:
    // Endpoint Descriptor //
    0x07,                       //the size of this descriptor //
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,                   //EndpointAddress
    _INT,                       //Attributes
    0x08,0x00,                  //size (note: 2 bytes)
    0x02,                       //Interval

The first two parameters are self-explanatory. They specify the
length of this endpoint descriptor (7) and the descriptor type.
The next parameter identifies the endpoint, the definitions are
defined in usb_device.h and has the following naming
convention:
_EP<##>_<dir>
where ## is the endpoint number and dir is the direction of
transfer. The dir has the value of either 'OUT' or 'IN'.
The next parameter identifies the type of the endpoint. Available
options are _BULK, _INT, _ISO, and _CTRL. The _CTRL is not
typically used because the default control transfer endpoint is
not defined in the USB descriptors. When _ISO option is used,
addition options can be ORed to _ISO. Example:
_ISO|_AD|_FE
This describes the endpoint as an isochronous pipe with adaptive
and feedback attributes. See usb_device.h and the USB
specification for details. The next parameter defines the size of
the endpoint. The last parameter in the polling interval.

-------------------------------------------------------------------
Adding a USB String
-------------------------------------------------------------------
A string descriptor array should have the following format:

rom struct{byte bLength;byte bDscType;word string[size];}sdxxx={
sizeof(sdxxx),DSC_STR,<text>};

The above structure provides a means for the C compiler to
calculate the length of string descriptor sdxxx, where xxx is the
index number. The first two bytes of the descriptor are descriptor
length and type. The rest <text> are string texts which must be
in the unicode format. The unicode format is achieved by declaring
each character as a word type. The whole text string is declared
as a word array with the number of characters equals to <size>.
<size> has to be manually counted and entered into the array
declaration. Let's study this through an example:
if the string is "USB" , then the string descriptor should be:
(Using index 02)
rom struct{byte bLength;byte bDscType;word string[3];}sd002={
sizeof(sd002),DSC_STR,'U','S','B'};

A USB project may have multiple strings and the firmware supports
the management of multiple strings through a look-up table.
The look-up table is defined as:
rom const unsigned char *rom USB_SD_Ptr[]={&sd000,&sd001,&sd002};

The above declaration has 3 strings, sd000, sd001, and sd002.
Strings can be removed or added. sd000 is a specialized string
descriptor. It defines the language code, usually this is
US English (0x0409). The index of the string must match the index
position of the USB_SD_Ptr array, &sd000 must be in position
USB_SD_Ptr[0], &sd001 must be in position USB_SD_Ptr[1] and so on.
The look-up table USB_SD_Ptr is used by the get string handler
function.

-------------------------------------------------------------------

The look-up table scheme also applies to the configuration
descriptor. A USB device may have multiple configuration
descriptors, i.e. CFG01, CFG02, etc. To add a configuration
descriptor, user must implement a structure similar to CFG01.
The next step is to add the configuration descriptor name, i.e.
cfg01, cfg02,.., to the look-up table USB_CD_Ptr. USB_CD_Ptr[0]
is a dummy place holder since configuration 0 is the un-configured
state according to the definition in the USB specification.

********************************************************************/
 
/*********************************************************************
 * Descriptor specific type definitions are defined in:
 * usb_device.h
 *
 * Configuration options are defined in:
 * usb_config.h
 ********************************************************************/
#ifndef __USB_DESCRIPTORS_C
#define __USB_DESCRIPTORS_C
 
/** INCLUDES *******************************************************/
#include "GenericTypeDefs.h"
#include "usb_config.h"
#include "./USB/usb.h"
#include "./USB/usb_function_cdc.h"
#include "./USB/usb_function_hid.h"
#include "./USB/usb_function_audio.h"

/** CONSTANTS ******************************************************/

#define PID 0x1E00

/* Device Descriptor */
ROM USB_DEVICE_DESCRIPTOR device_dsc=
{
    0x12,                   // Size of this descriptor in bytes
    USB_DESCRIPTOR_DEVICE,  // DEVICE descriptor type
    0x0200,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    USB_EP0_BUFF_SIZE,      // Max packet size for EP0, see usb_config.h
    0x04D8,                 // Vendor ID
    PID,
    0x0100,                 // Device release number in BCD format
    0x01,                   // Manufacturer string index
    0x02,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};


#ifdef USB_USE_HID
    #define HID_CFG_LEN 0x20
    #define HID_NUM_INTF 1
#else
    #define HID_CFG_LEN 0
    #define HID_NUM_INTF 0
#endif
#ifdef USB_USE_CDC
    #define CDC_CFG_LEN 58
    #define CDC_NUM_INTF 2
#else
    #define CDC_CFG_LEN 0
    #define CDC_NUM_INTF 0
#endif
#ifdef USB_USE_AUDIO_CLASS
    #define AUDIO_CFG_LEN 0x5B
    #define AUDIO_NUM_INTF 2
    #define HID_INTF_ID_X (HID_INTF_ID+0x02)
#else
    #define AUDIO_CFG_LEN 0
    #define AUDIO_NUM_INTF 0
    #define HID_INTF_ID_X HID_INTF_ID
#endif

#define CFG_LEN (HID_CFG_LEN+CDC_CFG_LEN+AUDIO_CFG_LEN+0x09)
#define NUM_INTF (HID_NUM_INTF+CDC_NUM_INTF+AUDIO_NUM_INTF)




/* Configuration 1 Descriptor */
ROM BYTE configDescriptor1[]={
    /* Configuration Descriptor */
    0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
    CFG_LEN,0x00,         	// Total length of data for this cfg //shijas
    NUM_INTF,               // Number of interfaces in this cfg
    1,                      // Index value of this configuration
    0,                      // Configuration string index
    _DEFAULT | _SELF,       // Attributes, see usb_device.h
    50,                     // Max power consumption (2X mA)


#if defined(USB_USE_HID)
    // 0x20 bytes

    /* Interface Descriptor */
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    HID_INTF_ID,                      // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this intf
    HID_INTF,               // Class code
    0,     // Subclass code
    0,     // Protocol code
    0,                      // Interface string index

    /* HID Class-Specific Descriptor */
    0x09,//sizeof(USB_HID_DSC)+3,    // Size of this descriptor in bytes
    DSC_HID,                // HID descriptor type
    0x11,0x01,                 // HID Spec Release Number in BCD format (1.11)
    0x00,                   // Country Code (0x00 for Not supported)
    HID_NUM_OF_DSC,         // Number of class descriptors, see usbcfg.h
    DSC_RPT,                // Report descriptor type
    HID_RPT01_SIZE,0x00,//sizeof(hid_rpt01),      // Size of the report descriptor

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    HID_EP | _EP_IN,                   //EndpointAddress
    _INTERRUPT,                       //Attributes
    0x40,0x00,                  //size
    0x01,                        //Interval

    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    HID_EP | _EP_OUT,                   //EndpointAddress
    _INTERRUPT,                       //Attributes
    0x40,0x00,                  //size
    0x01,                        //Interval
#elif defined(USB_USE_CDC)
    // 58 bytes (0x3A)

    /* Interface Descriptor */
    9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    CDC_INTF1_ID,                      // Interface Number
    0,                      // Alternate Setting Number
    1,                      // Number of endpoints in this intf
    COMM_INTF,              // Class code
    ABSTRACT_CONTROL_MODEL, // Subclass code
    V25TER,                 // Protocol code
    0,                      // Interface string index

    /* CDC Class-Specific Descriptors */
    sizeof(USB_CDC_HEADER_FN_DSC),
    CS_INTERFACE,
    DSC_FN_HEADER,
    0x10,0x01,

    sizeof(USB_CDC_ACM_FN_DSC),
    CS_INTERFACE,
    DSC_FN_ACM,
    USB_CDC_ACM_FN_DSC_VAL,

    sizeof(USB_CDC_UNION_FN_DSC),
    CS_INTERFACE,
    DSC_FN_UNION,
    CDC_COMM_INTF_ID,
    CDC_DATA_INTF_ID,

    sizeof(USB_CDC_CALL_MGT_FN_DSC),
    CS_INTERFACE,
    DSC_FN_CALL_MGT,
    0x00,
    CDC_DATA_INTF_ID,

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP02_IN,_INT,CDC_INT_EP_SIZE,0x02,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP02_IN,            //EndpointAddress
    _INTERRUPT,                       //Attributes
    0x08,0x00,                  //size
    0x02,                       //Interval

    /* Interface Descriptor */
    9,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    CDC_INTF2_ID,                      // Interface Number
    0,                      // Alternate Setting Number
    2,                      // Number of endpoints in this intf
    DATA_INTF,              // Class code
    0,                      // Subclass code
    NO_PROTOCOL,            // Protocol code
    0,                      // Interface string index

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_OUT,_BULK,CDC_BULK_OUT_EP_SIZE,0x00,
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP03_OUT,            //EndpointAddress
    _BULK,                       //Attributes
    0x40,0x00,                  //size
    0x00,                       //Interval

    /* Endpoint Descriptor */
    //sizeof(USB_EP_DSC),DSC_EP,_EP03_IN,_BULK,CDC_BULK_IN_EP_SIZE,0x00
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    _EP03_IN,            //EndpointAddress
    _BULK,                       //Attributes
    0x40,0x00,                  //size
    0x00,                       //Interval
#endif


#ifdef USB_USE_AUDIO_CLASS
    // 0x5B bytes

    /* USB Microphone Standard AC Interface Descriptor	*/
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,      // INTERFACE descriptor type
    AUDIO_CONTROL_INTERFACE_ID,    // Interface Number
    0x00,                   	   // Alternate Setting Number
    0x00,                   	   // Number of endpoints in this intf
    AUDIO_DEVICE,      			   // Class code
    AUDIOCONTROL,				   // Subclass code
    0x00,  						   // Protocol code
    0x00,                      	   // Interface string index


    /* USB Microphone Class-specific AC Interface Descriptor  */
	0x09,						  // Size of this descriptor, in bytes.
	CS_INTERFACE,				  // CS_INTERFACE Descriptor Type
	HEADER,						  // HEADER descriptor subtype
	0x00,0x01,					  // Audio Device compliant to the USB Audio specification version 1.00
	0x1E,0x00,					  // Total number of bytes returned for the class-specific AudioControl interface descriptor.
								  // Includes the combined length of this descriptor header and all Unit and Terminal descriptors.
	0x01,						  // The number of AudioStreaming interfaces in the Audio Interface Collection to which this AudioControl interface belongs
	0x01,						  // AudioStreaming interface 1 belongs to this AudioControl interface.


	/*USB Microphone Input Terminal Descriptor */
	0x0C,						  // Size of the descriptor, in bytes
	CS_INTERFACE,				  // CS_INTERFACE Descriptor Type
	INPUT_TERMINAL,				  // INPUT_TERMINAL descriptor subtype
	ID_INPUT_TERMINAL,			  // ID of this Terminal.
	MICROPHONE,                   // Terminal is Microphone (0x01,0x02)
	0x00,						  // No association
	0x01,						  // One channel
	0x00,0x00,					  // Mono sets no position bits
	0x00,						  // Unused.
	0x00,						  // Unused.

	/* USB Microphone Output Terminal Descriptor */
	0x09,						  // Size of the descriptor, in bytes (bLength)
	CS_INTERFACE,				  // CS_INTERFACE Descriptor Type (bDescriptorType)
	OUTPUT_TERMINAL,			  // OUTPUT_TERMINAL descriptor subtype (bDescriptorSubtype)
	ID_OUTPUT_TERMINAL,			  // ID of this Terminal. (bTerminalID)
	USB_STREAMING,			      // USB Streaming. (wTerminalType
	0x00,						  // unused			(bAssocTerminal)
	ID_INPUT_TERMINAL,	          // From Input Terminal.(bSourceID)
	0x00,						  // unused  (iTerminal)

	/* USB Microphone Standard AS Interface Descriptor (Alt. Set. 0) */
	0x09,						  // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,	  // INTERFACE descriptor type (bDescriptorType)
	AUDIO_STREAMING_INTERFACE_ID, // Index of this interface. (bInterfaceNumber)
	0x00,						  // Index of this alternate setting. (bAlternateSetting)
	0x00,						  // 0 endpoints.	(bNumEndpoints)
	AUDIO_DEVICE,				  // AUDIO (bInterfaceClass)
	AUDIOSTREAMING,				  // AUDIO_STREAMING (bInterfaceSubclass)
	0x00,						  // Unused. (bInterfaceProtocol)
	0x00,						  // Unused. (iInterface)

	/* USB Microphone Standard AS Interface Descriptor (Alt. Set. 1) */
	0x09,				          // Size of the descriptor, in bytes (bLength)
	USB_DESCRIPTOR_INTERFACE,     // INTERFACE descriptor type (bDescriptorType)
	AUDIO_STREAMING_INTERFACE_ID, // Index of this interface. (bInterfaceNumber)
	0x01,						  // Index of this alternate setting. (bAlternateSetting)
	0x01,					 	  // 1 endpoint	(bNumEndpoints)
	AUDIO_DEVICE,				  // AUDIO (bInterfaceClass)
	AUDIOSTREAMING,				  // AUDIO_STREAMING (bInterfaceSubclass)
	0x00,					      // Unused. (bInterfaceProtocol)
	0x00,						  // Unused. (iInterface)

	/*  USB Microphone Class-specific AS General Interface Descriptor */
	0x07, 						  // Size of the descriptor, in bytes (bLength)
	CS_INTERFACE,				  // CS_INTERFACE Descriptor Type (bDescriptorType)
	AS_GENERAL,					  // GENERAL subtype (bDescriptorSubtype)
	ID_OUTPUT_TERMINAL,			  // Unit ID of the Output Terminal.(bTerminalLink)
	0x01,						  // Interface delay. (bDelay)
	0x01,0x00,					  // PCM Format (wFormatTag)

	/*  USB Microphone Type I Format Type Descriptor */
	0x0B,						 // Size of the descriptor, in bytes (bLength)
	CS_INTERFACE,				 // CS_INTERFACE Descriptor Type (bDescriptorType)
	FORMAT_TYPE,				 // FORMAT_TYPE subtype. (bDescriptorSubtype)
	0x01,						 // FORMAT_TYPE_I. (bFormatType)
	0x01,						 // One channel.(bNrChannels)
	0x02,						 // Two bytes per audio subframe.(bSubFrameSize)
	0x10,						 // 16 bits per sample.(bBitResolution)
	0x01,						 // One frequency supported. (bSamFreqType)
	//0x40,0x1F,0x00,				 // 8000Hz. (tSamFreq)
    //0x1D,0x29,0x00, // 10,525Hz
    0x22,0x56,0x00, // 21050Hz
    //0x44,0xAC,0x00,  // 44100Hz

	/*  USB Microphone Standard Endpoint Descriptor */
	0x09,					    // Size of the descriptor, in bytes (bLength)
	0x05,						// ENDPOINT descriptor (bDescriptorType)
	AS_EP | _EP_IN,						// IN Endpoint 1. (bEndpointAddress)
	0x01,						// Isochronous, not shared. (bmAttributes)
	0x10,0x00,					// 16 bytes per packet (wMaxPacketSize)
	0x01,						// One packet per frame.(bInterval)
	0x00,						// Unused. (bRefresh)
	0x00,						// Unused. (bSynchAddress)

	/* USB Microphone Class-specific Isoc. Audio Data Endpoint Descriptor*/
	0x07,						// Size of the descriptor, in bytes (bLength)
	CS_ENDPOINT,				// CS_ENDPOINT Descriptor Type (bDescriptorType)
	AS_GENERAL,					// GENERAL subtype. (bDescriptorSubtype)
	0x00,						// No sampling frequency control, no pitch control, no packet padding.(bmAttributes)
	0x00,						// Unused. (bLockDelayUnits)
	0x00,0x00,					// Unused. (wLockDelay)
#endif

};



//Language code string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[1];}sd000={
sizeof(sd000),USB_DESCRIPTOR_STRING,{0x0409}};

//Manufacturer string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[11];}sd001={
sizeof(sd001),USB_DESCRIPTOR_STRING,
{'V','i','s','c','T','r','o','n','i','c','s'}};

//Product string descriptor
ROM struct{BYTE bLength;BYTE bDscType;WORD string[19];}sd002={
sizeof(sd002),USB_DESCRIPTOR_STRING,
{'H','e','x','L','i','g','h','t',' ','C','o','n','t','r','o','l','l','e','r'}
};

#ifdef USB_USE_HID
//Class specific descriptor - HID
ROM struct{BYTE report[HID_RPT01_SIZE];}hid_rpt01={
{
    0x06, 0x00, 0xFF,       // Usage Page = 0xFF00 (Vendor Defined Page 1)
    0x09, 0x01,             // Usage (Vendor Usage 1)
    0xA1, 0x01,             // Collection (Application)
    0x19, 0x01,             //      Usage Minimum
    0x29, 0x40,             //      Usage Maximum 	//64 input usages total (0x01 to 0x40)
    0x15, 0x01,             //      Logical Minimum (data bytes in the report may have minimum value = 0x00)
    0x25, 0x40,      	  	//      Logical Maximum (data bytes in the report may have maximum value = 0x00FF = unsigned 255)
    0x75, 0x08,             //      Report Size: 8-bit field size
    0x95, 0x40,             //      Report Count: Make sixty-four 8-bit fields (the next time the parser hits an "Input", "Output", or "Feature" item)
    0x81, 0x00,             //      Input (Data, Array, Abs): Instantiates input packet fields based on the above report size, count, logical min/max, and usage.
    0x19, 0x01,             //      Usage Minimum
    0x29, 0x40,             //      Usage Maximum 	//64 output usages total (0x01 to 0x40)
    0x91, 0x00,             //      Output (Data, Array, Abs): Instantiates output packet fields.  Uses same report size and count as "Input" fields, since nothing new/different was specified to the parser since the "Input" item.
    0xC0}                   // End Collection
};
#endif

//Array of configuration descriptors
ROM BYTE *ROM USB_CD_Ptr[]=
{
    (ROM BYTE *ROM)&configDescriptor1
};
//Array of string descriptors
ROM BYTE *ROM USB_SD_Ptr[USB_NUM_STRING_DESCRIPTORS]=
{
    (ROM BYTE *ROM)&sd000,
    (ROM BYTE *ROM)&sd001,
    (ROM BYTE *ROM)&sd002
};

#pragma code
#endif
/** EOF usb_descriptors.c ****************************************************/
