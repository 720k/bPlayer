#!/usr/bin/env bash
#porta="$1"
#portb="$2"
#echo "Bind UNIX sockets: $porta <--> $portb"
#[ -z $portb ] && printf "I need two port name\n"|| exit

outpipe="/tmp/socket_server_outpipe"
inpipe="/tmp/socket_server_inpipe"
[ -p "$outpipe" ] && rm $outpipe
[ -p "$inpipe" ] && rm $inpipe

mkfifo $outpipe
mkfifo $inpipe

socat - unix-listen:/tmp/frontend,fork > $outpipe < $inpipe &
socat - unix-listen:/tmp/backend,fork < $outpipe > $inpipe &
