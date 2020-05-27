# bPlayer
bPlayer Frontend / Backend

Status: Working prototype, very early stage

Test Configuration: 
1) Linux OS, Direct connection  (local socket), Backend <--> Frontend
2) Conduit connection (Linux Host / Windows Guest) Backend <--> Remote-Viewer <--> ConduitD <--> Frontend
  - Backend <--> Remote-Viewer patched : port automatically guessed by Backend
  - Remote-Viewer <--> ConduitD : specified in VM XML description
  - ConduitD <--> Frontend : conduit Listen on localhost:60999, frontend connect as tcp client
