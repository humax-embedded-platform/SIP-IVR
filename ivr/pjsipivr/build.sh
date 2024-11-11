#!/bin/sh
#if LD_LIBRARY_PATH doesn't contain /usr/local/lib -> export


export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
gcc -o ivr_app main.c -lpjsua -lpjsip-ua -lpjsip-simple -lpjsip -lpjmedia -lpjmedia-codec -lpjmedia-videodev -lpjmedia-audiodev -lpjmedia -lpjlib-util -lpj -lm -lpthread