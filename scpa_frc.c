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
#include <errno.h>
#include "crc32.c"
#include "base64.c"
 
#define porta     6100
#define MAXLINE 2048 

int main() {
	FILE* arq;
    int sockfd, i, j, k, l, crc, contVirg=0, len, n, tamArq;
    char buffer[MAXLINE], conteudoArquivo[MAXLINE], *decRecebido, crcRecebido[MAXLINE], Linha[MAXLINE], *base64out;
    char crcstr[MAXLINE], tamstr[MAXLINE], crcHX[MAXLINE] = {"0x"}, nomeRecebido[MAXLINE], *nome64, msgEnv[MAXLINE] = {};
    struct sockaddr_in servaddr, cliaddr;
     
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Falha na criação do socket!");
        exit(EXIT_FAILURE);
    }
     
    servaddr.sin_family    = AF_INET; // IPv4
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(porta);
     
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0){
        perror("Falha de conexão!");
        exit(EXIT_FAILURE);
    }
     
    len = sizeof(cliaddr);  
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
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
    	
		crc = xcrc32(decRecebido, strlen(decRecebido), 0);
    	snprintf(crcstr, MAXLINE, "%x", crc);
    	
    	strcat(crcHX, crcstr);
    	
    	if(strcmp(crcRecebido, crcHX) == 0){    		
    		strcat(msgEnv, "res,0,");
    		strcat(msgEnv, crcHX);
    		strcat(msgEnv, ",");
    		strcat(msgEnv, nome64);  
    		
    		for(i=0;i<strlen(nomeRecebido);i++){
	  			if(nomeRecebido[i] == '.'){
	  				nomeRecebido[i] = '\0';
	  			}
	  		}
    		
    		arq = fopen(strcat(nomeRecebido,".bin"), "wb");

	  	    if (arq == NULL){  // Se houve erro na abertura
		 		perror("Problemas na abertura do arquivo\n");
		 		return EXIT_FAILURE;
	  		}
	  		
	  		fwrite(decRecebido, sizeof(char), strlen(decRecebido), arq);
	  		   		   		
		} else {
			strcat(msgEnv, "res,-1,0xFFFFFFFF");
			strcat(msgEnv, ",");
			strcat(msgEnv, nome64);   		
		}   
		
		sendto(sockfd, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
			
    } else if(buffer[0] == 'r'){
    	k=0;
    
		for(i=0;i<n;i++){
			if(buffer[i] == ','){
				contVirg++;
			}

			if(contVirg == 4){
				nomeRecebido[k] = buffer[i+1];
				k++;
			} 			
		}
		
		decRecebido = base64_decode(nomeRecebido);
		
		arq = fopen(decRecebido, "rb");

  	    if (arq == NULL){  // Se houve erro na abertura
     		perror("Problemas na abertura do arquivo\n");
     		strcat(msgEnv, "res,-1,0xFFFFFFFF,");
     		strcat(msgEnv, nomeRecebido);
     		sendto(sockfd, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len);
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
    	
    	base64out = base64_encode(Linha);
    	
    	strcat(msgEnv, "env,");
    	strcat(msgEnv, tamstr);
    	strcat(msgEnv, ",");
    	strcat(msgEnv, "0x");
    	strcat(msgEnv, crcstr);
    	strcat(msgEnv, ",");
    	strcat(msgEnv, decRecebido);
    	strcat(msgEnv, ",");
    	strcat(msgEnv, base64out); 
    	
    	sendto(sockfd, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &cliaddr, len); 
    	
    	do{
    		n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    		buffer[n] = '\0';
    	}while(buffer[0] != 'p');
		    
    }
     
    return 0;
}
