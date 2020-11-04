:: This script splits an image by cue file and converts output to desired format
:: Parameters:
:: %1 - cue file
:: %2 - path to write output
:: %3 - input file
:: %4 - output file ext
@echo off
SETLOCAL ENABLEEXTENSIONS
SETLOCAL ENABLEDELAYEDEXPANSION

set bitrate=320K

if not exist shntool.exe (
  echo.ERROR: No shntool.exe found. Script aborted.
  goto:eof
)

echo.Splitting %3 by %1 into %2 ...
:: First we try to decode input file if ffmpeg is installed
set cueFile=%1
set outputDir=%2
set inputFile=%3

:: Removing quotes 
for /f "useback tokens=*" %%a in ('!cueFile!') do set cueFile=%%~a
for /f "useback tokens=*" %%a in ('!outputDir!') do set outputDir=%%~a
for /f "useback tokens=*" %%a in ('!inputFile!') do set inputFile=%%~a


echo.Ffmpeg found. First decoding input file into temp.wav...

ffmpeg -y -v warning -i "!inputFile!" "!outputDir!\temp.wav"
set inputFile="!outputDir!\temp.wav"
echo.Done

echo.Splitting temp.wav using shntool
shntool split -P dot -f %1 -d %2 -O always !inputFile!
:: removing temporary file
del !inputFile!
echo.Done
echo.

echo.Converting files into %4 ...
for %%i in ("!outputDir!"\split-track*.wav) do (
  echo.Converting %%i ...
  set file=%%i
  set file=!file:wav=!
  if %4==alac (
    ffmpeg -y -v warning -i "%%i" -acodec alac "!file!m4a"
  ) else if %4==ogg (
    ffmpeg -y -v warning -i "%%i" -acodec libvorbis -ab !bitrate! "!file!%4"
  ) else ffmpeg -y -v warning -i "%%i" -ab !bitrate! "!file!%4")

echo.Done
echo.

echo.Removing temporary files ...
del /F /Q "!outputDir!"\split-track*.wav
echo.Done


goto :eof
