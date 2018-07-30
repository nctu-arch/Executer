// ADXL345 Sensitivity(from datasheet) => 4mg/LSB   1G => 1000mg/4mg = 256 steps
// Tested value : 248
#define GRAVITY 248  //this equivalent to 1G in the raw data coming from the accelerometer 
#define Accel_Scale(x) x*(GRAVITY/9.81)//Scaling the raw data of the accel to actual acceleration in meters for seconds square

#define ToRad(x) (x*0.01745329252)  // *pi/180
#define ToDeg(x) (x*57.2957795131)  // *180/pi

// LPR530 & LY530 Sensitivity (from datasheet) => (3.3mv at 3v)at 3.3v: 3mV/ยบ/s, 3.22mV/ADC step => 0.93
// Tested values : 0.92
#define Gyro_Gain_X 0.92 //X axis Gyro gain
#define Gyro_Gain_Y 0.92 //Y axis Gyro gain
#define Gyro_Gain_Z 0.92 //Z axis Gyro gain
#define Gyro_Scaled_X(x) x*ToRad(Gyro_Gain_X) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Y(x) x*ToRad(Gyro_Gain_Y) //Return the scaled ADC raw data of the gyro in radians for second
#define Gyro_Scaled_Z(x) x*ToRad(Gyro_Gain_Z) //Return the scaled ADC raw data of the gyro in radians for second

#define Kp_ROLLPITCH 0.02
#define Ki_ROLLPITCH 0.00002
#define Kp_YAW 1.2
#define Ki_YAW 0.00002

/*For debugging purposes*/
//OUTPUTMODE=1 will print the corrected data, 
//OUTPUTMODE=0 will print uncorrected data of the gyros (with drift)
#define OUTPUTMODE 1

//#define PRINT_DCM 0     //Will print the whole direction cosine matrix
#define PRINT_ANALOGS 0 //Will print the analog raw data
#define PRINT_EULER 1   //Will print the Euler angles Roll, Pitch and Yaw

#define ADC_WARM_CYCLES 50
#define STATUS_LED 13 

extern int sensors[3] ;
extern int SENSOR_SIGN[9];

extern float G_Dt;

extern long timer;
extern long timer_old;
extern long timer24;
extern int AN[6]; //array that store the 3 ADC filtered data (gyros)
extern int AN_OFFSET[6];
extern int ACC[3];          //array that store the accelerometers data

extern int accel_x;
extern int accel_y;
extern int accel_z;
extern int magnetom_x;
extern int magnetom_y;
extern int magnetom_z;
extern int gyro_x;
extern int gyro_y;
extern int gyro_z;
extern float MAG_Heading;

extern float Accel_Vector[3];
extern float Gyro_Vector[3];
extern float Omega_Vector[3];
extern float Omega_P[3];
extern float Omega_I[3];
extern float Omega[3];

// Euler angles
extern float roll;
extern float pitch;
extern float yaw;

extern float errorRollPitch[3];
extern float errorYaw[3];

extern unsigned int counter;
extern int gyro_sat;

extern float DCM_Matrix[3][3];
extern float Update_Matrix[3][3];


extern float Temporary_Matrix[3][3];

void Normalize(void);
void Drift_correction(void);
void Accel_adjust(void);
void Matrix_update(void);
void Euler_angles(void);
void Compass_Heading(void);
void Matrix_Multiply(float a[3][3], float b[3][3],float mat[3][3]);
float Vector_Dot_Product(float vector1[3],float vector2[3]);
void Vector_Cross_Product(float vectorOut[3], float v1[3],float v2[3]);
void Vector_Scale(float vectorOut[3],float vectorIn[3], float scale2);
void Vector_Add(float vectorOut[3],float vectorIn1[3], float vectorIn2[3]);

