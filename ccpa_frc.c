//Trabalho 01: Fundamentos de Redes
//Pedro Lucas - 13/0035581
//Vinicius
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/cdefs.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "crc32.c"
#include "base64.c"
 
#define porta     6100
#define MAXLINE 2048

int main(int argc, char *argv[]) {
	FILE* arq;
    unsigned int n_sock, n, tamanho, tamArq, crc, entInt;
    char buffer[MAXLINE], tamstr[MAXLINE], crcstr[MAXLINE], msgEnv[MAXLINE], *base64out, Linha[MAXLINE];
    struct sockaddr_in end_serv;
//                        Address Family   Datagram
    if ((n_sock = socket(AF_INET,         SOCK_DGRAM, 0)) < 0 ) {
        perror("Falha na criacao do socket");
        exit(EXIT_FAILURE);
    }
    
    end_serv.sin_family = AF_INET;
    end_serv.sin_port = htons(porta);
    end_serv.sin_addr.s_addr = INADDR_ANY;
    
    /*
    *	Escrever aqui o codigo do trabalho utilizando as funções de enviar e receber abaixo!!!
    */
    
    if(strcmp(argv[1],"-e") == 0){
    	arq = fopen(argv[2], "rb");

  	    if (arq == NULL){  // Se houve erro na abertura
     		perror("Problemas na abertura do arquivo\n");
     		return EXIT_FAILURE;
  		}
  		
  		fseek(arq, 0, SEEK_END);
    	tamArq = ftell(arq);//Tamanho do arquivo %d
    	fseek(arq, 0, SEEK_SET);
    	
    	fgets(Linha, MAXLINE, arq);  // o 'fgets' lê até 99 caracteres ou até o '\n'
      	Linha[strlen(Linha)-1] = '\0'; 	
    	crc = xcrc32(Linha, strlen(Linha), 0);// crc32 em %d
    	
    	snprintf(tamstr, MAXLINE, "%d", tamArq);//converte tamArq de inteiro para string
    	snprintf(crcstr, MAXLINE, "%x", crc);//converte crc de inteiro para string   
    	//entInt = atoi(entBin);
    	
    	base64out = base64_encode(Linha);	
    	
    	strcat(msgEnv, "env,");
    	strcat(msgEnv, tamstr);
    	strcat(msgEnv, ",");
    	strcat(msgEnv, "0x");
    	strcat(msgEnv, crcstr);
    	strcat(msgEnv, ",");
    	strcat(msgEnv, argv[2]);
    	strcat(msgEnv, ",");
    	strcat(msgEnv, base64out);   	
    }
     
    sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
         
    n = recvfrom(n_sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho);
    buffer[n] = '\0';
    printf("%s\n", buffer);
 
    close(n_sock);
    fclose(arq);
    return 0;
}
