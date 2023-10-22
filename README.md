## CHAT WITH SOCKETS USING C
A chat broadcast system with options to set name, sent private messages and chat anonymous!

## How to run
### Server
- Compile and Start server before receive connections
  
  `gcc server.c -o server` and `./server`
- Now the server is running on port 5000 and waiting for clients

### Client
- Compile and start a client (you can choose some server IP and a name)
  `gcc client.c -o client`
- Starting client without name
  `./client 127.0.0.1`
- Starting client with a name
  `./client 127.0.0.1 usernameX` 
- Type `/help` at client console to see more options!
## Authors

- Leon Junio Martins Ferreira
- Edmar Melandes de Oliveira Junior
