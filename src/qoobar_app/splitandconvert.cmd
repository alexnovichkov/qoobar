REM This script splits an image by cue file and converts output to desired format
REM Parameters:
REM %1 - cue file
REM %2 - path to write output
REM %3 - input file
REM %4 - output file ext
@echo off

REM This script is a shortened version of Windows script splitandconvert.bat
REM because I have no OS/2 to check splitandconvert.bat for functioning in OS/2

shntool split -P dot -f %1 -d %2 -O always -o flac %3

goto :eof
