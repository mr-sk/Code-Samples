/********************************************************************* 
 * Author: ben sgro
 * 
 * Depends: Install the *nix phidget framework.
 *  	    Copy *.h into /usr/local/include.
 * 
 * Compile: cc -o ce_rfid ce_rfid.c -lphidget21
 * 
 ********************************************************************

#include <stdio.h>
#include <stdafx.h>
#include <pthread.h>
#include <phidget21.h> /* Provide by the Phidget framework. */

/*
 * 
 * Support Functions
 * 
 */


/* This function has to return an int because *_set_OnAttach_Handler
 * requires the argument be a pointer a function that returns an int.
 * It also requires we provide a pointer a user data structure.
 */
int HandleAttach(CPhidgetHandle RFIDHandle, void *userDSPtr)
{
	const char *deviceName;
	
	/* Returns a pointer to a null terminated buffer containing the device name. */
	CPhidget_getDeviceName(RFIDHandle, &deviceName);
	printf("%s attached.\n", deviceName);

	return 0;
}

/* This gets called if the device is unplugged or the connection is lost.
 * TODO: This should probably call a php file to send us an email.
 */
int HandleDetach(CPhidgetHandle RFIDHandle, void *userDSPtr)
{
	const char *deviceName;
	
	CPhidget_getDeviceName(RFIDHandle, &deviceName);
	printf("%s detached.\n", deviceName);

	return 0;
}


/* There is where the cool shit happens. When we read the tag, the function populates
 * the tagVal buffer. We can read out the RFID from that. Tags are a 10 character, 40bit
 * hexadecimal. 
 * 
 * We can also toggle the LED here. This provides visual feedback to the user. We will
 * turn the LED off when we loose the RFID.
 * 
 * Also, we DONT call our php script here. We wait until the reader losses the RFID,
 * and THEN we call the php script.
 */
int HandleRFIDTag(CPhidgetRFIDHandle RFIDHandle, void *userDSptr, unsigned char *tagVal)
{
	CPhidgetRFID_setLEDOn(RFIDHandle, 1); /* Turn the LED on. */
	
	printf("Tag Read: %02x%02x%02x%02x%02x\n",
				tagVal[0], tagVal[1], tagVal[2], tagVal[3], tagVal[4]);
	return 0;
}


/* This is where the even cooler shit happens. LostTag is called after an RFID tag is detected.
 * So, at this point, the LED is still on. We first turn it off, print out the tag id, 
 * and then execute our php script, passing the tag id as an argument.
 * 
 */
int HandleLostTag(CPhidgetRFIDHandle RFIDHandle, void *userDSptr, unsigned char *tagVal)
{
	char buff[22];

	CPhidgetRFID_setLEDOn(RFIDHandle, 0); /* Turn the LED off. */

	printf("Tag Lost: %02x%02x%02x%02x%02x\n",
				tagVal[0], tagVal[1], tagVal[2], tagVal[3], tagVal[4]);	

	/* Copy the entire command to the buffer. 
	 * BUFFSIZE = ./ (2) + purchase (8) + space (1) + 10 + NULL = 22
	 */
	snprintf(buff, 22, "./purchase %02x%02x%02x%02x%02x", tagVal[0], tagVal[1], tagVal[2], tagVal[3], tagVal[4]);
	
	system(buff); /* Call the php file w/the RFID as the argument. */
	
	return 0;
}


/*
 * 
 * MAIN
 * 
 */
int main(int argc, char* argv[])
{
	int returnVal = 0;
	
	/*
	 * Setup callbacks.
	 */
	
	/* Declare and create the handler. */
	CPhidgetRFIDHandle RFIDHandle = 0;
	returnVal = CPhidgetRFID_create(&RFIDHandle);
	if ( returnVal != EPHIDGET_OK ) { printf("Error _create\n"); return -1; }
	
	/* Provide a pointer to a callback function to run when the device is discovered. */
	returnVal = CPhidget_set_OnAttach_Handler( (CPhidgetHandle) RFIDHandle, HandleAttach, NULL);
	if ( returnVal != EPHIDGET_OK ) { printf("Error OnAttach_Handler\n"); return -1; }
	
	/* Provide a pointer to a callback function to run when the device is disconnected. */
	returnVal = CPhidget_set_OnDetach_Handler( (CPhidgetHandle) RFIDHandle, HandleDetach, NULL);
	if ( returnVal != EPHIDGET_OK) { printf("Error OnDetach_Handler\n"); return -1; }
	
	/* Provide a pointer to a callbank function to run when a RFID tag is read. */
	returnVal = CPhidgetRFID_set_OnTag_Handler(RFIDHandle, HandleRFIDTag, NULL);
	if ( returnVal != EPHIDGET_OK) { printf("Error onTag_Handler\n"); return -1; }

	/* ' ' ' ' ' ' ' ' ' ' ' ' ' is out of range. After a tag is indentified, within the
	 * next 500 milliseconds if no tag is found, the callback will be run. 
	 */
	returnVal = CPhidgetRFID_set_OnTagLost_Handler(RFIDHandle, HandleLostTag, NULL);
	if ( returnVal != EPHIDGET_OK) { printf("Error onTagLost_Handler\n"); return -1; }

	/*
	 * Connect to the physical device.
	 */
	returnVal = CPhidget_open( (CPhidgetHandle) RFIDHandle, -1);
	if ( returnVal != EPHIDGET_OK) { printf("Error _open\n"); return -1; }

	returnVal = CPhidget_waitForAttachment( (CPhidgetHandle) RFIDHandle, 10000); /* 10 seconds */
	if ( returnVal != EPHIDGET_OK) { printf("Error _waitForAttachment (Did you sudo?)\n"); return -1; }

	/* Turn the antenna on. */
	returnVal = CPhidgetRFID_setAntennaOn(RFIDHandle, 1);
	if ( returnVal != EPHIDGET_OK) { printf("Error _setAntennaOn\n"); return -1; }
	
	printf("Press any key to shutdown the reader.\n");
	/* This effectively causes the script to wait indefinetly. While waiting,
	 * any event that triggers our callbacks we handle. In the case of stdin,
	 * the app would shutdown.
	 */	
	getchar(); 
	
	/* Close it down. */
	CPhidget_close( (CPhidgetHandle) RFIDHandle);
	CPhidget_delete( (CPhidgetHandle) RFIDHandle);
	
	return 0;
}
/* newline. */
