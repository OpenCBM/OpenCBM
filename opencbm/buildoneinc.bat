@echo off
rem start buildoneinc

set BASH=bash

rem set CYGWIN=c:\cygwin
rem set CYGWINBIN=%CYGWIN%\BIN\
rem set BASH=%CYGWINBIN%bash

%BASH% %1/buildoneinc %1 %2
