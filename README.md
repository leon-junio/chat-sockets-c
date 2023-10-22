## CHAT WITH SOCKETS USING C
A broadcast chat system with options to set name, sent private messages and chat anonymous! Written in C with TCP Sockets.

## Requirements

- GCC
- Linux only (Tested with Ubuntu)
- Port 5000 free and unlocked by firewall
> Server is compatible with a remote network (AWS, AZURE or Google Cloud)

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
- Type anything and hit enter to sent to all other connected clients
  
## Authors

- [@Leon Junio](https://www.github.com/leon-junio)
- [@Vinicius Gabriel](https://www.github.com/ravixr)
- [@Edmar Melandes](https://www.github.com/Lexizz7)
