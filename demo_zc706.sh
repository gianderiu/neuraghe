#!/bin/bash 
./bin/resnet/main_zc706 -c ./weights_zc706/resnet -i ./demo_imgs/resnet/ -p 3 -h 224 -w 224 -o 1000 -P 50000 -b ./zynq/neuraghe-latest.bin
