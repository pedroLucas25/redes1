// Server side implementation of UDP client-server model
//https://www.geeksforgeeks.org/udp-server-client-implementation-c/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/cdefs.h>
#include "crc32.c"
#include "base64.c"
 
#define PORT     6100
#define MAXLINE 2048 

int main() {
	uint32_t decoMsg[MAXLINE], crc;
    int sockfd, i, j, k, l, len, n, contVirg=0;
    char buffer[MAXLINE], conteudoArquivo[MAXLINE], *decRecebido, crcRecebido[MAXLINE];
    char crcstr[MAXLINE], crcHX[MAXLINE] = {"0x"}, nomeRecebido[MAXLINE], *nome64, msgEnv[MAXLINE] = {};
    struct sockaddr_in servaddr, cliaddr;
     
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Falha na criação do socket!");
        exit(EXIT_FAILURE);
    }
    
    memset(&cliaddr, 0, sizeof(cliaddr)); 
     
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);
     
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("Falha de conexão!");
        exit(EXIT_FAILURE);
    }
     
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, ( struct sockaddr *) &cliaddr, &len);
    buffer[n] = '\0';
    //printf("%s\n", buffer);
    
    if(buffer[0] == 'e'){
    	j=0;
    	k=0;
    	l=0;
    	for(i=0;i<n;i++){
    		if(buffer[i] == ','){
    			contVirg++;
    		}
    		
    		if(contVirg == 2){
    			crcRecebido[j] = buffer[i+1];
    			j++;
    		}
    		
    		if(contVirg == 3){
    			nomeRecebido[k] = buffer[i+1];
    			k++;
    		}
    		
    		if(contVirg >= 4){
    			conteudoArquivo[l] = buffer[i+1];
    			l++;
    		}
    	}
    	
    	crcRecebido[strlen(crcRecebido)-1] = '\0';
    	nomeRecebido[strlen(nomeRecebido)-1] = '\0';
    	
    	nome64 = base64_encode(nomeRecebido);
    	
    	decRecebido = base64_decode(conteudoArquivo);
    	
    	//b64_decode(conteudoArquivo, strlen(conteudoArquivo), decoMsg);
		crc = xcrc32(decRecebido, strlen(decRecebido), 0);
    	snprintf(crcstr, MAXLINE, "%x", crc);
    	
    	strcat(crcHX, crcstr);
    	
    	if(strcmp(crcRecebido, crcHX) == 0){
    		strcat(msgEnv, "res,0,");
    		strcat(msgEnv, crcHX);
    		strcat(msgEnv, ",");
    		strcat(msgEnv, nome64);     		   		
    	} else {
    		strcat(msgEnv, "res,-1,0xFFFFFFFF");
    		strcat(msgEnv, ",");
    		strcat(msgEnv, nome64);   		
    	}   	
    }
    printf("%s\n", msgEnv);
    
    sendto(sockfd, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
     
    return 0;
}
