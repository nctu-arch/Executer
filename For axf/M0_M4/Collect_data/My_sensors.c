
#include "AHRS.h"
#include "Sensors_M0_batch.h"
#include "api.h"

void Read_Accel(int p){
//	if (sequenceComplete) {
//						sequenceComplete = 0;
//						rawADC = Chip_ADC_GetDataReg(LPC_ADC, BOARD_ADC_CH1);
//						if(rawADC & ADC_SEQ_GDAT_DATAVALID) AN[3] = ADC_DR_RESULT(rawADC);
//						rawADC = Chip_ADC_GetDataReg(LPC_ADC, BOARD_ADC_CH2);
//						if(rawADC & ADC_SEQ_GDAT_DATAVALID) AN[4] = ADC_DR_RESULT(rawADC);
//						rawADC = Chip_ADC_GetDataReg(LPC_ADC, BOARD_ADC_CH3);
//						if(rawADC & ADC_SEQ_GDAT_DATAVALID) AN[5] = ADC_DR_RESULT(rawADC);
//			}
			//printf("x=%d\ty=%d\tz=%d\n",x,y,z);
//    accel_x = SENSOR_SIGN[3] * (AN[3] - AN_OFFSET[3]);
//    accel_y = SENSOR_SIGN[4] * (AN[4] - AN_OFFSET[4]);
//    accel_z = SENSOR_SIGN[5] * (AN[5] - AN_OFFSET[5]);

//	batch[p][0] = SENSOR_SIGN[3] * (AN[3] - AN_OFFSET[3]);
//	batch[p][1] = SENSOR_SIGN[4] * (AN[4] - AN_OFFSET[4]);
//	batch[p][2] = SENSOR_SIGN[5] * (AN[5] - AN_OFFSET[5]);

	  shm_write(123,0);
	  shm_write(321,1);
	  shm_write(231,2);

    }
//################################################END OF ACCELEROMETER###########################################//




void Read_Gyro(int p){
//    AN[0]= ITG_read_X();
//    AN[1]= ITG_read_Y();
//    AN[2]= ITG_read_Z();
//    gyro_x = SENSOR_SIGN[0] * (AN[0] - AN_OFFSET[0]);
//    gyro_y = SENSOR_SIGN[1] * (AN[1] - AN_OFFSET[1]);
//    gyro_z = SENSOR_SIGN[2] * (AN[2] - AN_OFFSET[2]);

//    batch[p][3]= SENSOR_SIGN[0] * (AN[0] - AN_OFFSET[0]);
//    batch[p][4]= SENSOR_SIGN[1] * (AN[1] - AN_OFFSET[1]);
//    batch[p][5]= SENSOR_SIGN[2] * (AN[2] - AN_OFFSET[2]);

	  shm_write(123,3);
	  shm_write(321,4);
	  shm_write(231,5);

    }

void Read_Compass(int p){

	  int buf[3] = {0};
	  //Read_MAG(buf);

//     magnetom_y =  SENSOR_SIGN[6]*buf[0];
//     magnetom_x =  SENSOR_SIGN[7]*buf[1];
//     magnetom_z =  SENSOR_SIGN[8]*buf[2];

//	  batch[p][7] =  SENSOR_SIGN[6]*buf[0];
//	  batch[p][6] =  SENSOR_SIGN[7]*buf[1];
//	  batch[p][8] =  SENSOR_SIGN[8]*buf[2];

	  shm_write(123,6);
	  shm_write(321,7);
	  shm_write(231,8);

    }
//################################################END OF MAG###########################################//


