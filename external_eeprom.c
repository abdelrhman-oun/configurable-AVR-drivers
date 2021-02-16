/*--------------------------------------------------------------------------------------------------------------------------
 * file name  : eeprom.c
 * Author     : OUN
 * Created on : Dec 12, 2020
 * description: EEPROM source file driver
 --------------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------INCLUDES------------------------------------------------------------*/

#include "external_eeprom.h"

/*-----------------------------------------------FUNCTION DECLARATIONS-----------------------------------------------------*/

void EEPROM_init(TwiConfig *a_TwiConfig_Ptr){
	/* just initialize the I2C(TWI) module inside the MC */
	TWI_init(a_TwiConfig_Ptr);
}

uint8 EEPROM_writeOneByte(uint16 a_Address, uint8 a_Data){
	/* Send the Start Bit */
	TWI_start();
	if (TWI_getStatus() != TW_START)
		return ERROR;

	/* Send the device address, we need to get A8 A9 A10 address bits from the
	 * memory location address and R/W=0 (write) */
	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7)));
	if (TWI_getStatus() != TW_MT_SLA_W_ACK)
		return ERROR;

	/* Send the required memory location address */
	TWI_write((uint8)(a_Address));
	if (TWI_getStatus() != TW_MT_DATA_ACK)
		return ERROR;

	/* write byte to eeprom */
	TWI_write(a_Data);
	if (TWI_getStatus() != TW_MT_DATA_ACK)
		return ERROR;

	/* Send the Stop Bit */
	TWI_stop();

	return SUCCESS;
}

uint8 EEPROM_readOneByte(uint16 a_Address, uint8 *a_Data_Ptr){
	/* Send the Start Bit */
	TWI_start();
	if (TWI_getStatus() != TW_START)
		return ERROR;

	/* Send the device address, we need to get A8 A9 A10 address bits from the
	 * memory location address and R/W=0 (write) */
	TWI_write((uint8)((0xA0) | ((a_Address & 0x0700)>>7)));
	if (TWI_getStatus() != TW_MT_SLA_W_ACK)
		return ERROR;

	/* Send the required memory location address */
	TWI_write((uint8)(a_Address));
	if (TWI_getStatus() != TW_MT_DATA_ACK)
		return ERROR;

	/* Send the Repeated Start Bit */
	TWI_start();
	if (TWI_getStatus() != TW_REP_START)
		return ERROR;

	/* Send the device address, we need to get A8 A9 A10 address bits from the
	 * memory location address and R/W=1 (Read) */
	TWI_write((uint8)((0xA0) | ((a_Address & 0x0700)>>7) | 1));
	if (TWI_getStatus() != TW_MT_SLA_R_ACK)
		return ERROR;

	/* Read Byte from Memory without send ACK */
	*a_Data_Ptr = TWI_readWithNACK();
	if (TWI_getStatus() != TW_MR_DATA_NACK)
		return ERROR;

	/* Send the Stop Bit */
	TWI_stop();
	return SUCCESS;
}

uint8 EEPROM_writeFourBytes(uint16 a_Address, uint32 a_Data){
	uint8 i , data;
	for(i = 0; i < 4 ; i++){
		data = (a_Data>>(3-i)*8);
		EEPROM_writeOneByte((a_Address+i),data);
		_delay_ms(10);
	}
	return SUCCESS;

}
uint8 EEPROM_readFourBytes(uint16 a_Address, uint32 *a_Data_Ptr){
	uint8 i;
	uint32 readData;
	TWI_start();
	if (TWI_getStatus() != TW_START)
	{
		return ERROR;
	}

	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7))); /*slave address*/
	if (TWI_getStatus() != TW_MT_SLA_W_ACK)
	{
		return ERROR;
	}

	TWI_write((uint8)(a_Address)); /*word address*/
	if (TWI_getStatus() != TW_MT_DATA_ACK)
	{
		return ERROR;
	}

	TWI_start();
	if (TWI_getStatus() != TW_REP_START)
	{
		return ERROR;
	}

	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7))| 0x01); /*slave address*/
	if (TWI_getStatus() != TW_MT_SLA_R_ACK)
	{
		return ERROR;
	}

	*a_Data_Ptr = 0;  /*reading from user according to number of bits requested*/
	for(i = 0; i < 4 ; i++)
	{
		readData=0;
		if (i < 3) /*reading with ack all the time except last time*/
		{
			readData = TWI_readWithACK();

			if (TWI_getStatus() != TW_MR_DATA_ACK)
			{
				return ERROR;
			}
		}
		else
		{
			readData=TWI_readWithNACK();
			if (TWI_getStatus() != TW_MR_DATA_NACK)
			{
				return ERROR;
			}
		}
		/*putting first bit in MSB and the next bit in the next 8 bits and so on*/
		*a_Data_Ptr |= (readData<<((3-i)*8));
	}

	TWI_stop();
	return SUCCESS;
}

uint8 EEPROM_writeSixBytes(uint16 a_Address, uint8 *a_Data_Ptr){
	uint8 i , data;
	for(i = 0; i < 6 ; i++){
		data = a_Data_Ptr[i];
		EEPROM_writeOneByte((a_Address+i),data);
		_delay_ms(10);
	}
	return SUCCESS;

}
uint8 EEPROM_readSixBytes(uint16 a_Address, uint8 *a_Data_Ptr){
	uint8 i, readData;

	TWI_start();
	if (TWI_getStatus() != TW_START)
	{
		return ERROR;
	}

	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7))); /*slave address*/
	if (TWI_getStatus() != TW_MT_SLA_W_ACK)
	{
		return ERROR;
	}

	TWI_write((uint8)(a_Address)); /*word address*/
	if (TWI_getStatus() != TW_MT_DATA_ACK)
	{
		return ERROR;
	}

	TWI_start();
	if (TWI_getStatus() != TW_REP_START)
	{
		return ERROR;
	}

	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7))| 0x01); /*slave address*/
	if (TWI_getStatus() != TW_MT_SLA_R_ACK)
	{
		return ERROR;
	}

	for(i = 0; i < 6 ; i++){

		readData = 0;
		if (i < 5){ /*reading with ack all the time except last time*/

			readData = TWI_readWithACK();

			if (TWI_getStatus() != TW_MR_DATA_ACK)
			{
				return ERROR;
			}
		}
		else{
			readData = TWI_readWithNACK();
			if (TWI_getStatus() != TW_MR_DATA_NACK)
			{
				return ERROR;
			}
		}

		a_Data_Ptr[i] = readData;
	}
	TWI_stop();
	return SUCCESS;
}


uint8 EEPROM_writeBytes(uint16 a_Address, uint8 *a_Data_Ptr, uint8 a_numBytes){
	uint8 i , data;
	for(i = 0; i < a_numBytes ; i++){
		data = a_Data_Ptr[i];
		EEPROM_writeOneByte((a_Address+i),data);
		_delay_ms(10);
	}
	return SUCCESS;

}
uint8 EEPROM_readBytes(uint16 a_Address, uint8 *a_Data_Ptr, uint8 a_numBytes){
	uint8 i, readData;

	TWI_start();
	if (TWI_getStatus() != TW_START)
	{
		return ERROR;
	}

	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7))); /*slave address*/
	if (TWI_getStatus() != TW_MT_SLA_W_ACK)
	{
		return ERROR;
	}

	TWI_write((uint8)(a_Address)); /*word address*/
	if (TWI_getStatus() != TW_MT_DATA_ACK)
	{
		return ERROR;
	}

	TWI_start();
	if (TWI_getStatus() != TW_REP_START)
	{
		return ERROR;
	}

	TWI_write((uint8)(0xA0 | ((a_Address & 0x0700)>>7))| 0x01); /*slave address*/
	if (TWI_getStatus() != TW_MT_SLA_R_ACK)
	{
		return ERROR;
	}

	for(i = 0; i < a_numBytes ; i++){

		readData = 0;
		if (i < (a_numBytes-1)){ /*reading with ack all the time except last time*/

			readData = TWI_readWithACK();

			if (TWI_getStatus() != TW_MR_DATA_ACK)
			{
				return ERROR;
			}
		}
		else{
			readData = TWI_readWithNACK();
			if (TWI_getStatus() != TW_MR_DATA_NACK)
			{
				return ERROR;
			}
		}

		a_Data_Ptr[i] = readData;
	}
	TWI_stop();
	return SUCCESS;
}
