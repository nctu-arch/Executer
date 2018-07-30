
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "AHRS.h"


#define SAMPLE_RATE 1000 //ms
#define TOTAL_SAMPLE 1


char sequenceComplete = 0;

int sensors[3] = {1,2,0};  // Map the ADC channels gyro_x, gyro_y, gyro_z
int SENSOR_SIGN[9] = {-1,1,-1,1,1,1,-1,-1,-1};  //Correct directions x,y,z - gyros, accels, magnetormeter

float G_Dt=0.02;    // Integration time (DCM algorithm)  We will run the integration loop at 50Hz if possible

long timer=0;   //general purpuse timer
long timer_old;
long timer24=0; //Second timer used to print values
int AN[6] = {0,0,0,0,0,0}; //array that store the 3 ADC filtered data (gyros)
int AN_OFFSET[6]={0,0,0,0,0,0}; //Array that stores the Offset of the sensors
int ACC[3] = {0,0,0};          //array that store the accelerometers data

int accel_x = 0;
int accel_y = 0;
int accel_z = 0;
int magnetom_x = 0;
int magnetom_y = 0;
int magnetom_z = 0;
int gyro_x = 0;
int gyro_y = 0;
int gyro_z = 0;
float MAG_Heading = 0;

float Accel_Vector[3]= {0,0,0}; //Store the acceleration in a vector
float Gyro_Vector[3]= {0,0,0};//Store the gyros turn rate in a vector
float Omega_Vector[3]= {0,0,0}; //Corrected Gyro_Vector data
float Omega_P[3]= {0,0,0};//Omega Proportional correction
float Omega_I[3]= {0,0,0};//Omega Integrator
float Omega[3]= {0,0,0};

// Euler angles
float roll = 0;
float pitch = 0;
float yaw = 0;

float errorRollPitch[3]= {0,0,0};
float errorYaw[3]= {0,0,0};

unsigned int counter=0;
int gyro_sat=0;

float DCM_Matrix[3][3]= {
  {
    1,0,0  }
  ,{
    0,1,0  }
  ,{
    0,0,1  }
};
float Update_Matrix[3][3]={{0,1,2},{3,4,5},{6,7,8}}; //Gyros here


float Temporary_Matrix[3][3]={
  {
    0,0,0  }
  ,{
    0,0,0  }
  ,{
    0,0,0  }
};


int pbatch=0;

int count=0;

int batch_acc_x[200] = {0};
int batch_acc_y[200] = {0};
int batch_acc_z[200] = {0};
int batch_gyro_x[200] = {0};
int batch_gyro_y[200] = {0};
int batch_gyro_z[200] = {0};
int batch_mag_x[200] = {0};
int batch_mag_y[200] = {0};
int batch_mag_z[200] = {0};

int read_data(FILE* file, int* x0, int* y0, int* z0, int* x1, int* y1, int* z1, int* x2, int* y2, int* z2, int index) {

    int ch;

	while (1) {
    if (9 == fscanf(file, "%d,%d,%d,%d,%d,%d,%d,%d,%d", x0+index,y0+index,z0+index,x1+index,y1+index,z1+index,x2+index,y2+index,z2+index)) {
      return 1;
    }

    /* Advance to the next line */
    while ((ch = getc(file)) != '\n') {
      if (EOF == ch) {
	return 0;
      }
    }
  }

}

int main()
{

	int buf[3] = {0};
	int i=0;
	int loop_counter=0;
	FILE *fptr = NULL;
	char my_str[100];
	int index = 0;


	   fptr = fopen("/home/cws/app_input/ahrs_input.txt","r");
		 if(fptr==NULL)fprintf(stderr,"NO INPUT FILE!\n");

		//G_Dt = 1/SAMPLE_RATE;
		G_Dt = 0.0001;


		//*********Data acquisition******//

		for(i=0;i<TOTAL_SAMPLE;i++){
		read_data(fptr,batch_acc_x,batch_acc_y,batch_acc_z,batch_gyro_x,batch_gyro_y,batch_gyro_z,batch_mag_x,batch_mag_y,batch_mag_z,index);
		index++;
		//printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n",accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,magnetom_x,magnetom_y,magnetom_z);
		}

		//printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n",batch_acc_x[0],batch_acc_y[0],batch_acc_z[0],batch_gyro_x[0],batch_gyro_y[0],batch_gyro_z[0],batch_mag_x[0],batch_mag_y[0],batch_mag_z[0]);
		//printf("%d,%d,%d,%d,%d,%d,%d,%d,%d\n",batch_acc_x[1],batch_acc_y[1],batch_acc_z[1],batch_gyro_x[1],batch_gyro_y[1],batch_gyro_z[1],batch_mag_x[1],batch_mag_y[1],batch_mag_z[1]);

		//*************************//

		memcpy((char*)0x70000000,"a",sizeof(char));
		//************Calculations...when batch full********************************//
		  for(i=0;i<TOTAL_SAMPLE;i++){

				accel_x = batch_acc_x[i];
				accel_y = batch_acc_y[i];
				accel_z = batch_acc_z[i];
				gyro_x = batch_gyro_x[i];
				gyro_y = batch_gyro_y[i];
				gyro_z = batch_gyro_z[i];
				magnetom_x = batch_mag_x[i];
				magnetom_y = batch_mag_y[i];
				magnetom_z = batch_mag_z[i];

				//printf("accel_x = %d  , accel_y = %d  , accel_z=%d  . gyro_x = %d  , gyro_y = %d  , gyro_z = %d  , mag_x = %d  , mag_y = %d  , mag_z = %d  \n",accel_x,accel_y,accel_z,gyro_x,gyro_y,gyro_z,magnetom_x,magnetom_y,magnetom_z);
				  Compass_Heading();
				  Matrix_update();
				  Normalize();
				  Drift_correction();
				  Euler_angles();

				  printf("Roll = %f\tPitch = %f\tYaw=%f\n",roll,pitch,yaw);
			}
		memcpy((char*)0x70000000,"b",sizeof(char));

		fclose(fptr);
	return 0;
}


//int main(){
//
//	FILE *fptr = NULL;
//	char my_str[100];
//
//
//	memcpy((char*)0x70000000,"a",sizeof(char));
//
//
//	fptr = fopen("/home/cws/app_input/test.txt","r");
//	if(fptr==NULL)fprintf(stderr,"NO INPUT FILE!\n");
//	printf("Hello World\n");
//	while(fscanf(fptr,"%s",my_str)!=EOF){
//		printf("%s\n",my_str);
//	}
//
//	fclose(fptr);
//	memcpy((char*)0x70000000,"b",sizeof(char));
//	return 0;
//}
