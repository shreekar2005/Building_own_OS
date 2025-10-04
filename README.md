# Welcome to OOSS

In this repo you will find src code for completely new operating system called OOSS (idk what name should I give to our OS to I choose OOSS 😁).

OOSS is 32bit OS

Our actual goal is to develop OOSS. <br>
We are in learning phase also so there may be many subprojects like baby_OOSS, OOSS (our main goal project), busybox_linux, etc.

Currently this project is divided into many subproject as follows
1. ```baby_OOSS``` : for understanding how kernel loads in memory. In baby_OOSS we are loading bootloader in memory and then loading kernel from bootloder (with pure x86 assembly)
2. ```OOSS``` : our main goal OS to work on. It's major portion will be written in C++
3. ```busybox_linux``` : minimal linux based OS, which have many bash commands also (working). This is for understand how to build linux kernel

### You will find detailed README for subprojects if you open specific directory 

### Reference for learning : https://wiki.osdev.org/Expanded_Main_Page

My machine for development : Ubuntu 24.04.2 LTS
