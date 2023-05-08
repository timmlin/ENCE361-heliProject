/*
 *  Tim Lindbom (tli89)
 *  & Steph Post (spo88)
 *  group 55
 */



#ifndef ALTITUDE_H_
#define ALTITUDE_H_

void initADC (void);

void ADCIntHandler(void);

int32_t CalculateMeanADC();




#endif // ALTITUDE_H_
