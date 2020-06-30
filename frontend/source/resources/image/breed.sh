#!/usr/bin/env bash
source="bplayer"
convert -resize 32x32 -background none $source.svg  $source-32.png
convert -resize 64x64 -background none $source.svg  $source-64.png
convert -resize 128x128 -background none $source.svg  $source-128.png
convert -resize 256x256 -background none $source.svg  $source-256.png
convert -resize 64x64 -background none $source.svg  app.png
convert -resize 64x64 -background none $source.svg  app.ico