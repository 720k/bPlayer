#pragma once

namespace BPlayer {
    // local socket
    constexpr const char*   dataPortName =     "io.bplayer.data.0";
    constexpr const char *  streamPortName =   "io.bplayer.stream.0";
    //  or tcpip localhost:number socket
    constexpr const char *  locolhost =   "localhost";
    constexpr   int         dataPortNumber = 61000;
    constexpr   int         streamPortNumber = 61000;
}
