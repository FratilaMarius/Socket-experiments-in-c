#!/bin/bash

if [ "$1" = "c" ]; then 
  if gcc -o SMAIN_COMP Server_Main.c UI/cssfrm.c -IUI; then
    echo "Compilat."  
    if [ "$2" = "r" ]; then 
      chmod +x SMAIN_COMP
      ./SMAIN_COMP
    fi
  else
    echo "Are erori. Nu s-a compilat."
    exit 1
  fi
fi

if [ "$1" = "r" ]; then
  chmod +x SMAIN_COMP
  ./SMAIN_COMP
fi