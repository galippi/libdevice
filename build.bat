@echo off
cls
::path=C:\KBApps\DevEnv\Cygwin\V1_7_17\bin;%PATH%
::path=C:\KBApps\DevEnv\Cygwin\V2_3_1\bin;%PATH%
path=C:\Programok\cygwin\bin;%PATH%
rem set TMPDIR=c:\tmp
set WORK_ROOT=%CD%

bash --login -i -c "make -C $WORK_ROOT %1 %2 %3 %4"
