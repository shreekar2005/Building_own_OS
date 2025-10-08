# Welcome to Building_own_OS

## You will find detailed README in specific directory 
e.g) README for OSOS is in OSOS directory, README for cross-compiler is in cross-compiler directory

## You are Strongly Requested to checkout `./OSOS/README.md` :)
## Introduction
In this repo you will find src code for completely new operating system called "OSOS" (idk what name should I give to our OS to I choose "`OSOS (Open Source Operating System)`" 😁).
I am in learning phase also so there may be subprojects like baby_OSOS, busybox_linux, etc.

### Currently this Repository is divided as follows
1. `baby_OSOS` : for understanding how kernel loads in memory. In baby_OSOS we are loading bootloader in memory and then loading kernel from bootloder (with pure x86 assembly)
2. `cross-compiler` : You will need to build cross compiler in order to build OSOS
3. `OSOS` : actuall OS to work on. It's major portion will be written in C++. OSOS will be 32bit OS
4. `busybox_linux` : minimal linux based OS, which have many bash commands also (working). This is for understand how to build linux kernel

### Reference for learning : https://wiki.osdev.org/Expanded_Main_Page

My machine for development : `Ubuntu 24.04.2 LTS`
