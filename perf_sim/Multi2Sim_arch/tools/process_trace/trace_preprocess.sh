function data()
{
	if [ $2 == 0 ]; then			
		grep "^0  *" $1 > test_trace0
		awk '{
			previous = $3;
			print $2 " " $3 " " $4;
			while(getline==1)
			{
				now = $3;
				diff = now - previous;
				print $2 " " diff " " $4;
				previous = now;
			}
					  }' test_trace0 > test_C0 
		/home/yichengc/m2s/gen/Data_process test_C0 test_traceC0 &
	elif [ $2 == 1 ]; then
		grep "^1  *" $1 > test_trace1
		awk '{
			previous = $3;
			print $2 " " $3 " " $4;
			while(getline==1)
			{
				now = $3;
				diff = now - previous;
				print $2 " " diff " " $4;
				previous = now;
			}
					  }'  test_trace1 > test_C1 
		/home/yichengc/m2s/gen/Data_process test_C1 test_traceC1 &
	elif [ $2 == 2 ]; then
		grep "^2  *" $1 > test_trace2 
		awk '{
			previous = $3;
			print $2 " " $3 " " $4;
			while(getline==1)
			{
				now = $3;
				diff = now - previous;
				print $2 " " diff " " $4;
				previous = now;
			}
					  }' test_trace2 > test_C2 
		/home/yichengc/m2s/gen/Data_process test_C2 test_traceC2 &
	elif [ $2 == 3 ]; then
		grep "^3  *" $1 > test_trace3
		awk '{
			previous = $3;
			print $2 " " $3 " " $4;
			while(getline==1)
			{
				now = $3;
				diff = now - previous;
				print $2 " " diff " " $4;
				previous = now;
			}
					  }'  test_trace3 > test_C3 
		/home/yichengc/m2s/gen/Data_process test_C3 test_traceC3 &
	else
		echo "wrong data parameter"
	fi
}

function inst()
{
	if [ $2 == 0 ]; then
	grep "^0  *" $1 > test_traceI0 

	awk '{
			previous = $2;
			print  $2 " " $3;
			while(getline==1)
			{
				now = $2;
				diff = now - previous;
				print  diff " " $3;
				previous = now;
			}
					  }' test_traceI0 > test_I0  
	/home/yichengc/m2s/gen/Inst_preprocess test_I0 test_traceIC0 
	elif [ $2 == 1 ]; then
	grep "^1  *" $1 > test_traceI1 

	awk '{
			previous = $2;
			print  $2 " " $3;
			while(getline==1)
			{
				now = $2;
				diff = now - previous;
				print  diff " " $3;
				previous = now;
			}
					  }'  test_traceI1 > test_I1 
	/home/yichengc/m2s/gen/Inst_preprocess test_I1 test_traceIC1
	elif [ $2 == 2 ]; then
	grep "^2  *" $1 > test_traceI2 

	awk '{
			previous = $2;
			print  $2 " " $3;
			while(getline==1)
			{
				now = $2;
				diff = now - previous;
				print  diff " " $3;
				previous = now;
			}
					  }' test_traceI2 > test_I2 
	/home/yichengc/m2s/gen/Inst_preprocess test_I2 test_traceIC2
	elif [ $2 == 3 ]; then
	grep "^3  *" $1 > test_traceI3 
	awk '{
			previous = $2;
			print  $2 " " $3;
			while(getline==1)
			{
				now = $2;
				diff = now - previous;
				print  diff " " $3;
				previous = now;
			}
					  }'  test_traceI3 > test_I3 
	/home/yichengc/m2s/gen/Inst_preprocess test_I3 test_traceIC3 
	else
	echo "wrong inst parameter"
	fi
}

data $1 0 &
data $1 1 &
data $1 2 &
data $1 3 &


inst $2 0 &
inst $2 1 &
inst $2 2 &
inst $2 3 &





