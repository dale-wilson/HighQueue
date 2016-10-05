@REM Copyright (c) 2014 Object Computing, Inc.
@REM All rights reserved.
@REM See the file license.txt for licensing information.

REM Setting up HighQueue environment

@REM HighQueue depends on MPC V 3.6 or later. (http://www.ociweb.com/products/mpc)
@REM HQTest and HQPerformance depend on BOOST V 1.57.0 or later. (http://www.boost.org/) [earlier versions may work]
@REM Assumes Visual Studio installed in the default location (see VC_ROOT below) and VC..COMNTOOLS is set.

@REM Customize this file by setting variables to suit your environment
@REM Also you should customize HighQueue.features to enable particular features on your system.
@REM Delete the following line when you finishing customizing this file.
@echo See remarks in %0 for information about setting your build environment

@echo off

REM =====================================================================================
if "a" == "a%MPC_ROOT%" set MPC_ROOT=c:\MPC
if "a" == "a%BOOST_VERSION%" set BOOST_VERSION=boost_1_57_0
if "a" == "a%BOOST_ROOT%" set BOOST_ROOT=c:\boost\%BOOST_VERSION%
if "a" == "a%BOOST_LIB_DIRECTORY%" set BOOST_LIB_DIRECTORY=x64\lib
if "a" == "a%BOOST_LIB_PATH%" set BOOST_LIB_PATH=%BOOST_ROOT%\%BOOST_LIB_DIRECTORY%
REM END OF VALUES TO BE SET
REM =====================================================================================
REM Microsoft moved 32 bit apps to a new program files directory on 64 bit systems
set PROGRAM_FILES_X86=Program Files
if exist "C:\Program Files (x86)" set PROGRAM_FILES_X86=Program Files (x86)

REM Verify setup by checking for expected files/directories
set SETUP_CHECKING=MPC_ROOT=%MPC_ROOT%
if not exist "%MPC_ROOT%\mpc.pl" goto setup_is_bad

set SETUP_CHECKING=BOOST_ROOT=%BOOST_ROOT%
if not exist "%BOOST_ROOT%\boost" goto setup_is_bad

set SETUP_CHECKING=BOOST_LIB_PATH="%BOOST_LIB_PATH%"
if not exist "%BOOST_LIB_PATH%" goto setup_is_bad

REM Find visual studio.  
REM You can short-circuit this by setting VCVER before running this
REM However this also avoids the check to see if VC is installed in the expected place.
set SETUP_CHECKING=Setup checking visual studio common tools
if not "a" == "a%VCVER%" goto setup_is_ok

set VCVER=15
set SETUP_CHECKING=VS150COMNTOOLS=%VS150COMNTOOLS%
if exist "%VS150COMNTOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=14
set SETUP_CHECKING=VS140COMNTOOLS=%VS140COMNTOOLS%
if exist "%VS140COMNTOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=12
set SETUP_CHECKING=VS120COMNTOOLS=%VS120COMNTOOLS%
if exist "%VS120COMNTOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=11
set SETUP_CHECKING=VS110COMNTOOLS=%VS110COMNTOOLS%
if exist "%VS110COMNTOOLS%VSVARS32.BAT" goto setup_is_ok

set VCVER=10
set SETUP_CHECKING=VS100COMNTOOLS=%VS100COMNTOOLS%
if exist "%VS100COMNTOOLS%VSVARS32.BAT" goto setup_is_ok

REM goto setup_is_bad  (you are here) 

:setup_is_bad
ECHO Setup check failed: %SETUP_CHECKING%
ECHO Edit the setup.cmd file or change environment variables
goto end

:setup_is_ok
set SETUP_CHECKING=

set HighQueue_ROOT=%CD%

rem must be outside the if because of the parens in the directory name [what was microsoft thinking?]
if %VCVER%==15 goto isVC15
if %VCVER%==14 goto isVC14
if %VCVER%==12 goto isVC12
if %VCVER%==11 goto isVC11
if %VCVER%==10 goto isVC10
echo "UNKNOWN OR UNSUPPORTED VISUAL STUDIO VERSION: %VCVER%
goto end

:isVC10
:isVC11
:isVC12
:isVC14
:isVC15
set VC_ROOT=C:\%PROGRAM_FILES_X86%\Microsoft Visual Studio %VCVER%.0\VC\bin
call "%VC_ROOT%\VCVARS32.BAT" >nul

:vcIsSet

echo Found MPC at: %MPC_ROOT%
echo Found BOOST at: %BOOST_ROOT%
echo Found Visual Studio version %VCVER%

REM: This avoids growing PATH and INCLUDE every time setup is run
if "a" == "a%BASE_PATH%" set BASE_PATH=%PATH%
if "a" == "a%BASE_INCLUDE%" set BASE_INCLUDE=%INCLUDE%

set RELEASE64_PATH=%HighQueue_ROOT%\bin;%HighQueue_ROOT%\Output\Release;%MPC_ROOT%;%BOOST_ROOT%\x64\lib;%HighQueue_ROOT%\lib;%BASE_PATH%
set DEBUG64_PATH=%HighQueue_ROOT%\bin;%HighQueue_ROOT%\Output\Debug;%MPC_ROOT%;%BOOST_ROOT%\x64\lib;%HighQueue_ROOT%\lib;%BASE_PATH%
set RELEASE32_PATH=%HighQueue_ROOT%\bin;%HighQueue_ROOT%\Output\Release;%MPC_ROOT%;%BOOST_ROOT%\x86\lib;%HighQueue_ROOT%\lib;%BASE_PATH%
set DEBUG32_PATH=%HighQueue_ROOT%\bin;%HighQueue_ROOT%\Output\Debug;%MPC_ROOT%;%BOOST_ROOT%\x86\lib;%HighQueue_ROOT%\lib;%BASE_PATH%
set PATH=%RELEASE64_PATH%
set INCLUDE=%BOOST_ROOT%;%BASE_INCLUDE%

title HighQueue
:end


