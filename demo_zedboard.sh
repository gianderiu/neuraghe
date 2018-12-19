#!/bin/bash 
./bin/resnet/main_zed -c ./weights_zed/resnet -i ./demo_imgs/resnet/ -p 3 -h 224 -w 224 -o 1000 -P 40000 -b ./zynq/neuraghe_zed_4080.bin'
