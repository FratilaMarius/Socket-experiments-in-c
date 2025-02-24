#!/bin/bash

if [ "$1" = "c" ]; then 
  if gcc -o CMAIN_COMP Client_Main.c UI/cssfrm.c -IUI; then
    echo "Compilat."  
    if [ "$2" = "r" ]; then 
      chmod +x CMAIN_COMP
      ./CMAIN_COMP
    fi
  else
    echo "Are erori. Nu s-a compilat."
    exit 1
  fi
fi

if [ "$1" = "r" ]; then
  chmod +x CMAIN_COMP
  ./CMAIN_COMP
fi
