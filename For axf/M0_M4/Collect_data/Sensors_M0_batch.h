
#include <stdio.h>
//USed ADC channel
#define BOARD_ADC_CH1 3
#define BOARD_ADC_CH2 4
#define BOARD_ADC_CH3 8
extern char sequenceComplete;
extern int  rawADC;


void setup_ADC_for_ACC(void);



#define LPC_I2C_PORT               	LPC_I2C0
#define I2C_CLK_DIVIDER             2
#define I2C_BITRATE                 400000
#define I2C_BUFF_SZ                 256


int ITG_read_X(void);
int ITG_read_Y(void);
int ITG_read_Z(void);


void Init_I2C_PinMux(void);
void setupI2CMaster(void);




void MAG_setup(void);
void Read_MAG(int buf[3]);
void Read_Compass(int p);
void Read_Accel(int p);
void Read_Gyro(int p);

extern int SENSOR_SIGN[9];
extern int AN[6]; //array that store the 3 ADC filtered data (gyros)
extern int AN_OFFSET[6];
extern int ACC[3];          //array that store the accelerometers data

//sensor batch
extern int volatile batch[201][9];
extern int pbatch;
extern int bfull;


