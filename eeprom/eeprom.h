#ifndef __EEPROM_H
#define __EEPROM_H


int eeprom_init(void);
int eeprom_write(unsigned char addr, unsigned char* data, int len);
int eeprom_read(unsigned char addr, unsigned char* data, int len);


#endif
