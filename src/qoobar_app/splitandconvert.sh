#!/bin/bash
# This script splits an image by cue file and converts output to desired format
# Parameters:
# $1 - cue file
# $2 - path to write output
# $3 - input file
# $4 - ffmpeg codec
# $5 - output file ext

program_exists ()
{
  type "$1" &> /dev/null ;
}

main ()
{
set -e

local bitrate=320K

if ! program_exists shntool; then
  echo "\033[31mPlease install shntool\033[0m"
  exit 1
fi

local ffmpegProgram
if program_exists avconv; then
  ffmpegProgram=avconv
elif program_exists ffmpeg; then
  ffmpegProgram=ffmpeg
fi

local cue_file="$1"
local output_dir="$2"
local music_file="$3"
local fmt="$4"


if [ $ffmpegProgram ]; then
  #
  # Perform splitting to separate files
  #
  echo "\033[34m$ffmpegProgram found. First decoding input file into temp.wav...\033[0m"
  $ffmpegProgram -y -v warning -i "$music_file" "$output_dir"/temp.wav
  echo "\033[32mDone!\033[0m"

  echo "\033[34mSplitting $music_file ...\033[0m"
  shntool split -d "$output_dir" -f "$cue_file" -P dot -O always "$output_dir"/temp.wav
  rm -rf "$output_dir"/temp.wav
  echo "\033[32mDone!\033[0m"

  #
  # Convert each file to fmt
  #

  echo "\033[34mConverting splitted files to $fmt ...\033[0m"
  echo "Warning: Some file formats can be unavailable on your computer."
  for file in "$output_dir"/*.wav; do
    echo "\033[34mConverting $file ...\033[0m"
    local outfile="${file%.wav}.$fmt"
    if [ $fmt = "alac" ]; then
      $ffmpegProgram -y -v warning -i "$file" -acodec alac "${file%.wav}.m4a"
    elif [ $fmt = "ogg" ]; then
      $ffmpegProgram -y -v warning -i "$file" -acodec libvorbis -ab $bitrate "$outfile"
    else
      $ffmpegProgram -y -v warning  -i "$file" -strict experimental -ab $bitrate "$outfile"
    fi
  done
  echo "\033[32mDone!\033[0m"

  #
  # Delete temp stuff
  #

  echo "\033[34mRemoving temporary files ...\033[0m"
  rm -rf "$output_dir"/split-track*.wav
else
  echo "\033[34mSplitting $music_file ...\033[0m"
  shntool split -d "$output_dir" -f "$cue_file" -P dot -O always -o flac "$music_file"
fi
echo "\033[32mDone!\033[0m"
}

main "$@"

