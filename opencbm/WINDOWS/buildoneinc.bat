@echo off
rem start buildoneinc

if not defined BASH set BASH=bash

rem set CYGWIN=c:\cygwin
rem set CYGWINBIN=%CYGWIN%\BIN\
rem set BASH=%CYGWINBIN%bash

%BASH% %1/WINDOWS/buildoneinc %1/WINDOWS %2
