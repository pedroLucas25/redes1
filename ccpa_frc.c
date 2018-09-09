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
    int n_sock, n, tamanho, tamArq, crc, j, k, i, l, m, contVirg=0, tOut=0;
    char buffer[MAXLINE], tamstr[MAXLINE], crcstr[MAXLINE], msgEnv[MAXLINE] = {}, *base64out, Linha[MAXLINE], crcRecebido[MAXLINE];
    char conteudoArquivo[MAXLINE], *decRecebido, crcHX[MAXLINE] = {"0x"}, nomeRecebido[MAXLINE], er[MAXLINE];
    struct sockaddr_in end_serv;
//                        Address Family   Datagram
    if ((n_sock = socket(AF_INET,         SOCK_DGRAM, 0)) < 0 ) {
        perror("Falha na criacao do socket");
        exit(EXIT_FAILURE);
    }
    
    end_serv.sin_family = AF_INET;
    end_serv.sin_port = htons(porta);
    end_serv.sin_addr.s_addr = INADDR_ANY;
//////////////////////////////////////////////Concluído/////////////////////////////////////////////////////////////////////////////////////    
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
    	
    	do{
    		sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
    		if((n = recvfrom(n_sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho))<=0){tOut++;}
    		else {break;}
    		sleep(1);
			if(tOut>=3){
				printf("Erro! Tempo excedido!");
				return 1;
			}
		}while(tOut<=3);
		
    	buffer[n] = '\0';
    	k=0;j=0;
		
		for(i=0;i<n;i++){
			if(buffer[i] == ','){
				contVirg++;
			}

			if(contVirg == 1){
				er[k] = buffer[i+1];
				k++;
			}
			
			if(contVirg == 2){
    			crcRecebido[j] = buffer[i+1];
    			j++;
    		}		
		}
		
		er[strlen(er)-1] = '\0';
		crcRecebido[strlen(crcRecebido)-1] = '\0';
		
		strcat(crcHX, crcstr);
		
		if(strcmp(crcHX, crcRecebido) != 0){
			do{
				sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
				if((n = recvfrom(n_sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho))<=0){
					tOut++;
					buffer[n] = '\0';
					k=0;j=0;
					
					for(i=0;i<n;i++){
						if(buffer[i] == ','){
							contVirg++;
						}
						
						if(contVirg == 2){
							crcRecebido[j] = buffer[i+1];
							j++;
						}		
					}
					crcRecebido[strlen(crcRecebido)-1] = '\0';
				}
				else {break;}
				sleep(1);
				if(tOut>=3){
					printf("Erro! Tempo excedido!");
					return 1;
				}
			}while(tOut<=3 || strcmp(crcHX,crcRecebido)!=0);		
		}
    	
    	if(strcmp(er,"-1") == 0){printf("Ocorreu um erro no servidor!\n");return 1;}
    	
    	printf("Opercao concluida com sucesso!\n");
    	    	
    	fclose(arq);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    	 	
    } else if(strcmp(argv[1],"-r") == 0){    
    	base64out = base64_encode(argv[2]);
    	
    	strcat(msgEnv, "rec,0,0xFFFFFFFF,,");
    	strcat(msgEnv, base64out);
    	
    	do{
    		sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
    		if((n = recvfrom(n_sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho))<=0){tOut++;}
    		else {break;}
    		sleep(1);
			if(tOut>=3){
				printf("Erro! Tempo excedido!");
				return 1;
			}
		}while(tOut<=3);
		
    	do{
    		buffer[n] = '\0';
			j=0;l=0;k=0;m=0;
			
			for(i=0;i<n;i++){
				if(buffer[i] == ','){
					contVirg++;
				}
				
				if(contVirg == 1){
					er[m] = buffer[i];
					m++;
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
			er[strlen(er)-1] = '\0';
			
			if(strcmp(er,"-1") == 0){printf("Ocorreu um erro no servidor!\n");return 1;}
			
			decRecebido = base64_decode(conteudoArquivo);
			crc = xcrc32(decRecebido, strlen(decRecebido), 0);
			snprintf(crcstr, MAXLINE, "%x", crc);
			strcat(crcHX, crcstr);
			
			for(i=0;i<strlen(nomeRecebido);i++){
	  			if(nomeRecebido[i] == '.'){
	  				nomeRecebido[i] = '\0';
	  			}
	  		}
	  		
	  		if(strcmp(crcRecebido,crcHX) != 0){
	  			sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
	  			n = recvfrom(n_sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho);  
	  			tOut++;		
	  		} else {break;}
	  		
	  		if(tOut>=3){
				printf("Erro! Tempo excedido!");
				return 1;
			}
    	
    	}while(strcmp(crcRecebido,crcHX) != 0 || tOut<=3);
    	
    	arq = fopen(strcat(nomeRecebido,".bin"), "wb");

  	    if (arq == NULL){  // Se houve erro na abertura
	 		perror("Problemas na abertura do arquivo\n");
	 		return EXIT_FAILURE;
  		}
  		
  		fwrite(decRecebido, sizeof(char), strlen(decRecebido), arq);
  		
  		msgEnv[0] = '!';
  		
  		sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
  		
  		printf("Operacao concluida com sucesso!\n");
  		
  		fclose(arq); 
/////////////////////////////////////////////////////////Concluído////////////////////////////////////////////////////////////////////////////////////// 		   	
    } else if(strcmp(argv[1],"-c") == 0){ 
    	base64out = base64_encode(argv[2]);
    	
    	strcat(msgEnv, "cal,0,0xFFFFFFFF,,");
    	strcat(msgEnv, base64out);
    	
    	do{
    		sendto(n_sock, (const char *)msgEnv, strlen(msgEnv), MSG_CONFIRM, (const struct sockaddr *) &end_serv, sizeof(end_serv));
    		if((n = recvfrom(n_sock, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &end_serv, &tamanho))<=0){tOut++;}
    		else {break;}
    		sleep(1);
			if(tOut>=3){
				printf("Erro! Tempo excedido!");
				return 1;
			}
		}while(tOut<=3);
		
		buffer[n] = '\0'; 
		k=0;j=0;
		
		for(i=0;i<n;i++){
			if(buffer[i] == ','){
				contVirg++;
			}

			if(contVirg == 1){
				er[k] = buffer[i+1];
				k++;
			}
			
			if(contVirg == 2){
    			crcRecebido[j] = buffer[i+1];
    			j++;
    		}
		
		}
		
		er[strlen(er)-1] = '\0';
		crcRecebido[strlen(crcRecebido)-1] = '\0';
		
		if(strcmp(er,"-1") == 0){printf("Ocorreu um erro no servidor!\n");return 1;}
		   	
    	printf("CRC32 calculado pelo servidor: %s\n", crcRecebido);
    
    } else {printf("Opcao digitada invalida!!!\n");}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    close(n_sock);
    return 0;
}
