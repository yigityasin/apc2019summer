#12 16 24 32 40 48 
#!/bin/bash

size=96000000
t_num=(1 2 4 8 12 16 24 32 40 48)
lock_t=("A" "P" "T")

for t in "${t_num[@]}"
	do
		for l in "${lock_t[@]}"
		do
			for j in $(seq 1 10);
			do 			
				./increment $size $t $l;	
			done
		done
	 done


