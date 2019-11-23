/*
 * OPL.h
 *
 * Created: 04.02.2019 20:32:21
 *  Author: DEADMAN
 */ 


#ifndef OPL_H_
#define OPL_H_

void opl_init();

void opl_reset();

void opl_write(unsigned char address, unsigned char data);

#endif /* OPL_H_ */