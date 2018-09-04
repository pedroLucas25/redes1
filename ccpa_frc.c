//Trabalho 01: Fundamentos de Redes
//Pedro Lucas - 13/0035581
//Vinicius
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
 
#define PORT     6100

int main(int argc, char *argv[]) {
    int n_sock, n, tamanho;
    char buffer[2048];
    char *mensagem = argv[1];
    struct sockaddr_in end_serv;
//                        Address Family   Datagram
    if ( (n_sock = socket(AF_INET,         SOCK_DGRAM, 0)) < 0 ) {
        perror("Falha na criacao do socket");
        exit(EXIT_FAILURE);
    }
    
    /*
    *	Escrever aqui o codigo do trabalho utilizando as funções de enviar e receber abaixo!!!
    */
    
    end_serv.sin_family = AF_INET;
    end_serv.sin_port = htons(PORT);
    end_serv.sin_addr.s_addr = INADDR_ANY;
     
    sendto(n_sock, (const char *)mensagem, strlen(mensagem), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
    //printf("mensagem message sent.\n");
         
    n = recvfrom(n_sock, (char *)buffer, 2048, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho);
    buffer[n] = '\0';
    //printf("Server : %s\n", buffer);
 
    close(n_sock);
    return 0;
}
