#!/bin/bash
#===============================================================================
#
#          FILE:  step1_spec2006.sh
# 
#         USAGE:  ./step1_spec2006.sh 
# 
#   DESCRIPTION:  
# 
#       OPTIONS:  ---
#  REQUIREMENTS:  ---
#          BUGS:  ---
#         NOTES:  ---
#        AUTHOR:  Porshe , john740207@gmail.com
#       COMPANY:  ---
#       VERSION:  1.0
#       CREATED:  12/21/2010 10:26:02 AM PDT
#      REVISION:  ---
#      MODIFIED:  IC (Yin-Chi Peng), psycsieic@gmail.com, 2011_01_14
#
#===============================================================================

#Benchmark=(barnes  cholesky  fft  fmm  lu_cb  lu_ncb  ocean_cp  radiosity  radix  raytrace  water-nsquared  water-spatial)
#Benchmark=(bodytrack ferret streamcluster swaptions)
#Benchmark=(470.lbm)
Benchmark=(cholesky)
cores=(1)

program_count=0
n=1
limit=2


#if [ "$#" -eq 3 ]; then
 # ./step1_exp_parsec.sh $1 $2 $3 $4 &

#elif [ "$#" -eq 0 ]; then
  for benchmark in ${Benchmark[@]}
  do
    for core in ${cores[@]}
    do
      for job in `jobs -p`
      do
        program_count=$((${program_count}+${n}))
      done

      while [ $program_count -ge $limit ]
      do
        program_count=0
        for job in `jobs -p`
        do
          program_count=$((${program_count}+${n}))
        done
      done

      ./step1_exp_spec2006.sh ${benchmark} ${core} ${prefetch_degree} ${prefetch_distance} $1 &

    done
  done

#else
#  echo "./step1_parsec.sh for all, or"
 # echo "./step1_parsec.sh [Benchmark] [Core] [Prefetch_Degree] [Prefetch_distance]"
 # echo "[Benchmark]: blackscholes  bodytrack  canneal  dedup  facesim  ferret  fluidanimate  freqmine  raytrace  streamcluster  swaptions  vips  x264"
 # echo "[Core]: 4, 8, 16"
 # echo "[Prefetch_Degree]: 0, 1, 2, 4"
 # echo "[Prefetch_distance]: 1, 2"
#fi
