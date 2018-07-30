//------------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited.
//
//            (C) COPYRIGHT 2010 ARM Limited.
//                ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited.
//
//      SVN Information
//
//      Checked In          : 2010-08-03 14:15:21 +0100 (Tue, 03 Aug 2010)
//
//      Revision            : 144883
//
//      Release Information : AT510-MN-80001-r0p0-00rel0
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Cortex-M0 DesignStart C program example
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Include standard C library and RealView Compiler header.
//------------------------------------------------------------------------------

#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


//------------------------------------------------------------------------------
// Implement the minimum number of functions required to support
// standard C input/output operations without a debugger attached.
//------------------------------------------------------------------------------

// Define the location of the output console in the DesignStart testbench.
volatile unsigned char *console = (volatile unsigned char *) 0x40000000U;

// Implement a simple structure for C's FILE handle.
struct __FILE { int handle; };
FILE __stdout;
FILE __stdin;



// Implement file IO handling, only console output is supported.
time_t time(time_t* t)        { static time_t clock = 0; return clock++; }
int fputc(int ch, FILE *f)    { *console = ch; return ch; }
int ferror(FILE *f)           { return 0; }
int fgetc(FILE *f)            { return -1; }
int __backspace(FILE *stream) { return 0; }
void _ttywrch(int ch)         { fputc(ch,&__stdout); }



// Writing 0xD to the console causes the DesignStart testbench to finish.
void _sys_exit(void)          { fputc(0x0D,&__stdout); while(1); }
//------------------------------------------------------------------------------
// 														ROW DATA
//------------------------------------------------------------------------------


double row_data[1000] = {
1.079     ,
1.14562   ,
1.0625    ,
1.0638    ,
1.14206   ,
1.07026   ,
1.11716   ,
1.11393   ,
1.04924   ,
1.07479   ,
1.07382   ,
1.05409   ,
1.02434   ,
1.05215   ,
1.02531   ,
1.05409   ,
1.11231   ,
1.10713   ,
1.11328   ,
1.04633   ,
1.04989   ,
1.03825   ,
1.08029   ,
1.09937   ,
1.12201   ,
1.11781   ,
1.10746   ,
1.0211    ,
1.04213   ,
1.0211    ,
1.0127    ,
1.08708   ,
1.0942    ,
1.08029   ,
1.10099   ,
1.02531   ,
1.02822   ,
1.11942   ,
1.11651   ,
1.10843   ,
1.10843   ,
1.10778   ,
1.09064   ,
1.02822   ,
1.08191   ,
1.01011   ,
1.08094   ,
1.00655   ,
1.03048   ,
1.09323   ,
1.07932   ,
1.01755   ,
1.08741   ,
1.06218   ,
1.10713   ,
1.0211    ,
1.09323   ,
1.03889   ,
1.09905   ,
1.00784   ,
1.05409   ,
1.03242   ,
0.997495  ,
1.00655   ,
1.00558   ,
1.01981   ,
1.08935   ,
1.04827   ,
1.09905   ,
1.02693   ,
1.01787   ,
1.10293   ,
1.0528    ,
1.08482   ,
1.03663   ,
1.08094   ,
1.09064   ,
1.08644   ,
1.06994   ,
1.00881   ,
1.09032   ,
1.00914   ,
1.0072    ,
1.02854   ,
1.0997    ,
1.09775   ,
1.10843   ,
1.01884   ,
1.10196   ,
1.01916   ,
1.0473    ,
1.09905   ,
1.07512   ,
1.08352   ,
1.00623   ,
1.01431   ,
1.08126   ,
1.09129   ,
1.02337   ,
1.09258  ,
1.02369  ,
1.08805  ,
1.02402  ,
1.09808  ,
1.03048  ,
1.01302  ,
1.10034  ,
1.08741  ,
1.00946  ,
1.02725  ,
1.06671  ,
0.999759 ,
1.0114   ,
1.0625   ,
0.995555 ,
1.07835  ,
1.08773  ,
1.06897  ,
1.02046  ,
1.09614  ,
1.03436  ,
1.05312  ,
1.09743  ,
1.07188  ,
1.06509  ,
1.09193  ,
1.08061  ,
0.994584 ,
1.02531  ,
1.02272  ,
0.995555 ,
1.08191  ,
1.00914  ,
1.01981  ,
1.01043  ,
1.00299  ,
1.0059   ,
1.09711  ,
1.01949  ,
1.07867  ,
1.03242  ,
1.08255  ,
1.00558  ,
1.07382  ,
0.991997 ,
0.99232  ,
1.06897  ,
1.03986  ,
1.04827  ,
0.995878 ,
0.994908 ,
1.01205  ,
1.068    ,
0.994261 ,
1.07447  ,
1.00817  ,
1.07123  ,
0.986822 ,
1.07318  ,
1.0722   ,
1.06638  ,
1.01237  ,
1.02596  ,
0.973562 ,
1.0321   ,
1.05312  ,
1.06089  ,
1.06832  ,
1.02984  ,
1.04989  ,
0.99135  ,
0.99232  ,
1.00526  ,
1.06218  ,
1.01755  ,
0.97712  ,
0.977766 ,
0.986822 ,
0.975179 ,
1.05571  ,
0.969358 ,
0.991027 ,
0.978413 ,
0.998465 ,
1.03922  ,
1.06768  ,
1.01593  ,
0.996848 ,
0.985205 ,
1.05797  ,
0.985852 ,
0.977766 ,
1.05927  ,
0.969358 ,
0.976149 ,
1.00041  ,
1.03178  ,
0.985529 ,
0.988116 ,
1.04213  ,
1.01108  ,
0.993291 ,
1.03178  ,
1.01593  ,
1.01043  ,
1.01658  ,
1.01464  ,
0.984882 ,
0.986499 ,
0.990703 ,
0.971622 ,
0.970004 ,
0.963859 ,
0.964506 ,
1.04439  ,
0.994584 ,
1.00558  ,
1.06509  ,
1.02466  ,
0.972592 ,
1.05636  ,
0.998465 ,
1.06865  ,
0.97712  ,
1.06024  ,
1.05765  ,
0.985529 ,
0.966123 ,
0.964506 ,
1.03566  ,
1.0114   ,
1.00267  ,
0.96774  ,
1.05409  ,
1.05603  ,
0.973885 ,
1.06056  ,
0.998465 ,
1.05636  ,
1.04277  ,
1.04536  ,
0.964506 ,
1.05409  ,
1.0473   ,
1.00396  ,
0.999436 ,
0.957715 ,
0.975179 ,
0.989733 ,
0.974532 ,
0.968064 ,
0.989086 ,
1.06347  ,
1.04698  ,
1.01819  ,
1.0321   ,
1.01561  ,
0.969358 ,
1.04795  ,
0.962566 ,
1.03048  ,
1.04213  ,
0.957068 ,
0.97906  ,
0.978413 ,
1.04665  ,
1.04892  ,
1.04601  ,
0.972915 ,
1.00817  ,
1.02628  ,
1.0017   ,
1.04957  ,
0.975503 ,
1.0418   ,
1.05118  ,
0.958361 ,
1.04213  ,
0.981971 ,
1.03339  ,
1.03048  ,
1.03728  ,
1.04374  ,
1.01464  ,
0.997818 ,
0.971622 ,
1.01884  ,
0.975179 ,
1.04989  ,
1.0486   ,
1.04957  ,
1.03954  ,
1.05086  ,
0.975503 ,
0.975826 ,
1.04277  ,
0.980677 ,
1.04277  ,
1.04601  ,
0.963536 ,
0.97809  ,
1.05539  ,
1.04019  ,
0.974856 ,
1.00202  ,
1.03048  ,
1.02596  ,
0.980677 ,
1.0473   ,
0.973885 ,
1.01237  ,
1.03307  ,
1.04407  ,
1.0321   ,
1.03178  ,
0.963213 ,
1.02369  ,
0.979707 ,
0.99232  ,
0.995555 ,
1.06089  ,
1.00202  ,
1.06606  ,
1.02531  ,
0.989086 ,
1.05733  ,
0.96677  ,
0.962566 ,
0.997495 ,
0.958038 ,
1.03436  ,
1.03954  ,
1.01173  ,
1.04213  ,
1.06186  ,
1.05991  ,
1.05215  ,
1.00396  ,
1.0528   ,
0.975503 ,
0.987469 ,
0.972268 ,
1.05312  ,
0.973239 ,
1.05312  ,
0.962889 ,
0.987146 ,
0.96483  ,
1.04989  ,
0.974532 ,
1.02466  ,
1.06444  ,
0.983588 ,
1.07059  ,
1.00526  ,
0.997495 ,
1.00526  ,
0.983911 ,
0.987792 ,
0.985852 ,
0.973562 ,
0.972268 ,
1.01043  ,
0.97712  ,
1.05991  ,
1.00914  ,
0.983911 ,
1.02531  ,
1.0224   ,
1.06315  ,
1.06638  ,
1.08094  ,
1.07318  ,
1.03275  ,
1.07059  ,
0.995878 ,
1.06929  ,
0.983911 ,
1.06541  ,
1.06606  ,
1.04245  ,
0.983265 ,
0.986175 ,
1.07609  ,
1.00881  ,
1.00235  ,
1.05733  ,
1.08967  ,
1.06832  ,
1.08449  ,
1.00138  ,
1.07026  ,
1.07738  ,
1.07609  ,
1.00978  ,
0.992644 ,
1.07576  ,
0.988116 ,
0.990703 ,
1.079    ,
1.08741  ,
1.09549  ,
1.02822  ,
1.07609  ,
1.10164  ,
1.03922  ,
1.0735   ,
1.01625  ,
1.00558  ,
1.0887   ,
1.0722   ,
1.04019  ,
1.04407  ,
1.057    ,
1.08708  ,
1.01852  ,
1.079    ,
1.02693  ,
1.06735  ,
1.06315  ,
1.02143  ,
1.09484  ,
1.02757  ,
1.08482  ,
1.01981  ,
1.02305  ,
1.06865  ,
1.03792  ,
1.03436  ,
1.09226  ,
1.03178  ,
1.00817  ,
1.04277  ,
1.08482  ,
1.05506  ,
1.11166  ,
1.07382  ,
1.04989  ,
1.05797  ,
1.07091  ,
1.03501  ,
1.03889  ,
1.10455  ,
1.03728  ,
1.08611  ,
1.07706  ,
1.04019  ,
1.10843  ,
1.06735  ,
1.04601  ,
1.05054  ,
1.06186  ,
1.03857  ,
1.06186  ,
1.04568  ,
1.10649  ,
1.10843  ,
1.04342  ,
1.06153  ,
1.09646  ,
1.10519  ,
1.03242  ,
1.04374  ,
1.07382  ,
1.11942  ,
1.08449  ,
1.04213  ,
1.08126  ,
1.05668  ,
1.08094  ,
1.05571  ,
1.06606  ,
1.06347  ,
1.10616  ,
1.04665  ,
1.05474  ,
1.12913  ,
1.1081   ,
1.06121  ,
1.1039   ,
1.12428  ,
1.1149   ,
1.05506  ,
1.13074  ,
1.06283  ,
1.08158  ,
1.12266  ,
1.14303  ,
1.13301  ,
1.05991  ,
1.05021  ,
1.13398  ,
1.11328  ,
1.06929  ,
1.13042  ,
1.10034  ,
1.13139  ,
1.04957  ,
1.12525  ,
1.12136  ,
1.12428  ,
1.1343   ,
1.10034  ,
1.08902  ,
1.07415  ,
1.079    ,
1.07932  ,
1.10099  ,
1.0832   ,
1.13851  ,
1.05054  ,
1.13948  ,
1.0984   ,
1.11716  ,
1.10875  ,
1.14562  ,
1.05959  ,
1.07479  ,
1.13204  ,
1.14497  ,
1.11328  ,
1.09581  ,
1.12913  ,
1.07123  ,
1.09873  ,
1.12331  ,
1.09064  ,
1.12816  ,
1.14756  ,
1.07285  ,
1.10293  ,
1.10422  ,
1.07415  ,
1.15209  ,
1.14336  ,
1.14886  ,
1.1592   ,
1.16115  ,
1.11813  ,
1.1592   ,
1.07285  ,
1.12039  ,
1.15112  ,
1.13883  ,
1.05765  ,
1.12783  ,
1.05668  ,
1.06121  ,
1.08773  ,
1.14465  ,
1.14627  ,
1.13624  ,
1.09517  ,
1.14659  ,
1.15823  ,
1.16276  ,
1.10455  ,
1.15953  ,
1.14174  ,
1.08999  ,
1.14724  ,
1.15306  ,
1.10584  ,
1.11878  ,
1.10325  ,
1.12719  ,
1.14077  ,
1.16373  ,
1.1508   ,
1.07964  ,
1.16567  ,
1.15112  ,
1.09549  ,
1.14724  ,
1.10293  ,
1.16212  ,
1.07641  ,
1.16244  ,
1.15985  ,
1.10843  ,
1.15241  ,
1.166    ,
1.09129  ,
1.10649  ,
1.08967  ,
1.13462  ,
1.14886  ,
1.14853  ,
1.10002  ,
1.11457  ,
1.17085  ,
1.10455  ,
1.17214  ,
1.17441  ,
1.12622  ,
1.10552  ,
1.1605   ,
1.11263  ,
1.17441  ,
1.14109  ,
1.09808  ,
1.18734  ,
1.11004  ,
1.1094   ,
1.15209  ,
1.10713  ,
1.18443  ,
1.19446  ,
1.17052  ,
1.17602  ,
1.09614  ,
1.16244  ,
1.10746  ,
1.10099  ,
1.10358  ,
1.11134  ,
1.14206  ,
1.12039  ,
1.19769  ,
1.11975  ,
1.12654  ,
1.20125  ,
1.19931  ,
1.17473  ,
1.20578  ,
1.12428  ,
1.12104  ,
1.10455  ,
1.19155  ,
1.1799   ,
1.19219  ,
1.16697  ,
1.19672  ,
1.18184  ,
1.20254  ,
1.21225  ,
1.19284  ,
1.13236  ,
1.19834  ,
1.2158   ,
1.19316  ,
1.2103   ,
1.12848  ,
1.18961  ,
1.19478  ,
1.11522  ,
1.17538  ,
1.14206  ,
1.14239  ,
1.14012  ,
1.13204  ,
1.14012  ,
1.20772  ,
1.17246  ,
1.15597  ,
1.2019   ,
1.21936  ,
1.16147  ,
1.2061   ,
1.13107  ,
1.20319  ,
1.13042  ,
1.12686  ,
1.19899  ,
1.12622  ,
1.20287  ,
1.1508   ,
1.14142  ,
1.21419  ,
1.16503  ,
1.14465  ,
1.15468  ,
1.21936  ,
1.20351  ,
1.2213   ,
1.18572  ,
1.12816  ,
1.21386  ,
1.14271  ,
1.19963  ,
1.12913  ,
1.13754  ,
1.19672  ,
1.21904  ,
1.13301  ,
1.22292  ,
1.13851  ,
1.19866  ,
1.15597  ,
1.22162  ,
1.17861  ,
1.21774  ,
1.21192  ,
1.19155  ,
1.13171  ,
1.13624  ,
1.17408  ,
1.15629  ,
1.10972  ,
1.12492  ,
1.12233  ,
1.12072  ,
1.19834  ,
1.12331  ,
1.13236  ,
1.12589  ,
1.11522  ,
1.16794  ,
1.1812   ,
1.17958  ,
1.12104  ,
1.17052  ,
1.11166  ,
1.10455  ,
1.14594  ,
1.08449  ,
1.10746  ,
1.17182  ,
1.10681  ,
1.09484  ,
1.10584  ,
1.16955  ,
1.08352  ,
1.09678  ,
1.11522  ,
1.07932  ,
1.15047  ,
1.11037  ,
1.07641  ,
1.0735   ,
1.12395  ,
1.05668  ,
1.07059  ,
1.12298  ,
1.13818  ,
1.11069  ,
1.06865  ,
1.13171  ,
1.11651  ,
1.09646  ,
1.08935  ,
1.05377  ,
1.10067  ,
1.07059  ,
1.12039  ,
1.03857  ,
1.03404  ,
1.11619  ,
1.11845  ,
1.11328  ,
1.1094   ,
1.05927  ,
1.03663  ,
1.11328  ,
1.09937  ,
1.11134  ,
1.1246   ,
1.08126  ,
1.04083  ,
1.02531  ,
1.10552  ,
1.10681  ,
1.01561  ,
1.08773  ,
1.01593  ,
1.10228  ,
1.03404  ,
1.0266   ,
1.03081  ,
1.03792  ,
1.11037  ,
1.03404  ,
1.07447  ,
1.11554  ,
1.04924  ,
1.1039   ,
1.03436  ,
1.01528  ,
1.10067  ,
1.07059  ,
1.08611  ,
1.10293  ,
1.10261  ,
1.0321   ,
1.05054  ,
1.02628  ,
1.06541  ,
1.07706  ,
1.11231  ,
1.03663  ,
1.03178  ,
1.03242  ,
1.09581  ,
1.01949  ,
1.00978  ,
1.02143  ,
1.03501  ,
1.05345  ,
1.01528  ,
1.0431   ,
1.02854  ,
1.02207  ,
1.02951  ,
1.06735  ,
1.04471  ,
1.10034  ,
1.10519  ,
1.01787  ,
1.01496  ,
1.0114   ,
1.08741  ,
0.996848 ,
1.07964  ,
1.04504  ,
1.08999  ,
1.08417  ,
1.01076  ,
1.01916  ,
1.01431  ,
1.09873  ,
1.00655  ,
1.0169   ,
1.0266   ,
1.08352  ,
1.04051  ,
1.01237  ,
1.00364  ,
1.01593  ,
1.03663  ,
0.995555 ,
1.08191  ,
1.01658  ,
1.04245  ,
1.00073  ,
1.08547  ,
1.02531  ,
1.0887   ,
1.02628  ,
1.03339  ,
1.09743  ,
1.08967  ,
1.06218  ,
1.08126  ,
1.08611  ,
1.01755  ,
1.06962  ,
0.993614 ,
1.07253  ,
1.01949  ,
1.06218  ,
1.0583   ,
1.05183  ,
1.02337  ,
1.0942   ,
1.0929   ,
1.07609  ,
1.01334  ,
1.08514  ,
0.998142 ,
1.01625  ,
1.0528   ,
1.04116  ,
1.04245  ,
1.07285  ,
1.00041  ,
0.990703 ,
1.08417  ,
1.03695  ,
1.00461  ,
1.08741  ,
1.02046  ,
1.00978  ,
1.00396  ,
1.06477  ,
1.08352  ,
1.08288  ,
1.00914  ,
0.994584 ,
0.993937 ,
1.03275  ,
0.984235 ,
1.02175  ,
1.05571  ,
1.05474  ,
1.05894  ,
1.06089  ,
1.01431  ,
1.06735  ,
1.01949  ,
1.00332  ,
1.01658  ,
1.02822  ,
0.983265 ,
0.976473 ,
1.03728  ,
1.0583   ,
0.980354 ,
1.05151  ,
1.0583   ,
0.973562 ,
0.996525 ,
1.02693  ,
1.01787  ,
1.0431   ,
0.987469 ,
0.989086 ,
0.985852 ,
1.04439  ,
1.06671  ,
0.97712  ,
0.972268 ,
1.03857  ,
1.05151  ,
0.991027 ,
0.970975 ,
0.966123 ,
0.976149 ,
1.05894  ,
0.989086 ,
0.982618 ,
0.984558 ,
1.04245  ,
1.02951  ,
1.05409  ,
0.97712  ,
0.993937 ,
0.967417 ,
0.972915 ,
1.0486   ,
1.01819  ,
0.97712  ,
1.01367  ,
1.03728  ,
0.966447 ,
0.983265 ,
1.0473   ,
1.06929  ,
0.999436 ,
1.02434  ,
1.01722  ,
1.00008  ,
1.06509  ,
0.972915 ,
0.975503 ,
1.02272  ,
0.982294 ,
0.975826 ,
1.01108  ,
0.9658   ,
1.04665  ,
0.986822 ,
1.057    ,
1.03566  ,
0.981971 ,
1.05054  ,
0.989733 ,
0.978413 ,
1.00461  ,
0.995555 ,
1.05636  ,
1.04957  ,
0.972915 ,
0.976796 ,
0.990056 ,
0.964506 ,
1.04277  ,
0.985205 ,
1.05862  ,
0.971945 ,
0.99135  ,
0.998789 ,
0.979384 ,
0.987792 ,
0.977766 ,
0.97712  ,
1.04892  ,
1.05474  ,
1.04924  ,
1.01593  ,
0.994584 ,
0.964506 ,
1.03113  ,
0.984235 ,
0.973885 ,
0.990703 ,
1.06056  ,
0.975179 ,
1.01787  ,
1.00623  ,
1.04989  ,
0.990703 ,
1.00752  ,
1.03792  ,
1.04019  ,
1.05054  ,
0.970651 ,
1.04245  ,
0.998789 ,
0.989733 ,
1.02951  

	
};


//------------------------------------------------------------------------------
// 														YOUR DESIGN
//------------------------------------------------------------------------------

// maximum number of inputs that can be handled
// in one function call
#define MAX_INPUT_LEN   100
// maximum length of filter than can be handled
#define MAX_FLT_LEN     100
// buffer to hold all of the input samples
#define BUFFER_LEN      (MAX_FLT_LEN - 1 + MAX_INPUT_LEN)

#define FILTER_LEN  63
int16_t coeffs[ FILTER_LEN ] =
{
 -1468, 1058,   594,   287,    186,  284,   485,   613,
   495,   90,  -435,  -762,   -615,   21,   821,  1269,
   982,    9, -1132, -1721,  -1296,    1,  1445,  2136,
  1570,    0, -1666, -2413,  -1735,   -2,  1770,  2512,
  1770,   -2, -1735, -2413,  -1666,    0,  1570,  2136,
  1445,    1, -1296, -1721,  -1132,    9,   982,  1269,
   821,   21,  -615,  -762,   -435,   90,   495,   613,
   485,  284,   186,   287,    594, 1058, -1468
};
#define FILTER_LEN_MA   20

double coeffsMa[ FILTER_LEN_MA ] =
{
    0.05, 0.05, 0.05, 0.05, 0.05,
    0.05, 0.05, 0.05, 0.05, 0.05,
    0.05, 0.05, 0.05, 0.05, 0.05,
    0.05, 0.05, 0.05, 0.05, 0.05
};

// number of samples to read per loop
#define SAMPLES   105


// array to hold input samples
double insamp[ BUFFER_LEN ];

// FIR init
void firFloatInit( void )
{
    memset( insamp, 0, sizeof( insamp ) );
}

// the FIR filter function
void firFloat( double *coeffs, double *input, double *output, int length, int filterLength ){

   double acc;     // accumulator for MACs
   double *coeffp; // pointer to coefficients
   double *inputp; // pointer to input samples
   int n;
   int k;

   // put the new samples at the high end of the buffer

   memcpy( &insamp[filterLength - 1], input, length * sizeof(double) );

   // apply the filter to each input sample
   for ( n = 0; n < length; n++ ) {
   // calculate output n
      coeffp = coeffs;
      inputp = &insamp[filterLength - 1 + n];
      acc = 0;
         for ( k = 0; k < filterLength; k++ ) {
            acc += (*coeffp++) * (*inputp--);
         }
      output[n] = acc;
   }

   // shift input samples back in time for next time
   memmove( &insamp[0], &insamp[length], (filterLength - 1) * sizeof(double) );
}


int main(void) {
	
    int i, j;
    double input[SAMPLES];
    double output[SAMPLES];
    double data_output[1000];
   for(j = 0; j < 10; j++){

   // read samples from file

   for(i=0;i<SAMPLES;i++){
      input[i] = row_data[j+100*i];
   }
	
   // perform the filtering
   firFloat( coeffsMa, input, output, SAMPLES, FILTER_LEN_MA );

   // write samples to file
   for(i=0;i<SAMPLES;i++)
      data_output[j] = output[j+100*i];
   }
	return 0;
}
