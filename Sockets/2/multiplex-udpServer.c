/*Author: Abel Coronado*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

int main(int argc, char ** argv){

	struct addrinfo hints;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_flags = AI_PASSIVE;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM; //PROTOCOLO UDP

	struct addrinfo * res;

	int rc = getaddrinfo(argv[1], argv[2], &hints, &res);

	if(rc != 0){
		printf("Error getaddrinfo(): %s\n", gai_strerror(rc));
		return -1;
	}

	int udpServer = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	bind(udpServer, (struct sockaddr *) res->ai_addr, res->ai_addrlen);
	
	char buf[2];
	char host[NI_MAXHOST];
	char serv[NI_MAXSERV];	

	/*DATOS CLIENTE, LOS DEVOLVERÁ LA LLAMADA RECVFROM*/
	struct sockaddr_storage cliente_addr;
	socklen_t cliente_addrlen = sizeof(struct sockaddr_storage);

	fd_set readset;
	int result = -1;

	while(1){
		/*MULTIPLEXACIÓN*/
		do {
			FD_ZERO(&readset);
			FD_SET(0, &readset);
			FD_SET(udpServer, &readset);

			result = select(udpServer+1, &readset, NULL, NULL, NULL);
			
		}	while(result == -1);

		/*CLIENTE UDP*/
		time_t tiempo = time(NULL);
		struct tm *tm = localtime(&tiempo);
		size_t max;
		char s[50];
		if(FD_ISSET(udpServer, &readset)){
			ssize_t bytes = recvfrom(udpServer, buf, 2, 0, (struct sockaddr *) &cliente_addr, &cliente_addrlen);
			buf[1] = '\0';

			getnameinfo((struct sockaddr *) &cliente_addr, cliente_addrlen, 
				host, NI_MAXHOST, serv, NI_MAXSERV, NI_NUMERICHOST|NI_NUMERICSERV);

			printf("Recibí %ld byte(s) de %s(%s)\n", bytes, host, serv);


			if(buf[0] == 't'){
				
				ssize_t bytesT = strftime(s, max, "%I:%M:%S %p", tm);
				s[bytesT] = '\0';

				sendto(udpServer, s, bytesT, 0, (struct sockaddr *) &cliente_addr, cliente_addrlen);
				
			} else if (buf[0] == 'd'){
				
				ssize_t bytesT = strftime(s, max, "%A %d - %B - %Y", tm);
				s[bytesT] = '\0';

				sendto(udpServer, s, bytesT, 0, (struct sockaddr *) &cliente_addr, cliente_addrlen);

			} else if (buf[0] == 'q'){
				sendto(udpServer, "Comando de finalización recibido, saliendo...\0", 47,
				 0, (struct sockaddr *) &cliente_addr, cliente_addrlen);
				break;

			} else {
				sendto(udpServer, "Comando no soportado...\0", 26,
				 0, (struct sockaddr *) &cliente_addr, cliente_addrlen);
			}

		/*CONSOLA*/	
		} else {
			read(0, buf, 2);
			buf[1] = '\0';
			printf("Recibí %i byte(s) de consola\n", 2);


			if(buf[0] == 't'){
				ssize_t bytesT = strftime(s, max, "%I:%M:%S %p", tm);
				s[bytesT] = '\0';

				printf("%s\n", s);
			} else if(buf[0] == 'd'){
				ssize_t bytesT = strftime(s, max, "%A %d - %B - %Y", tm);
				s[bytesT] = '\0';

				printf("%s\n", s);
			} else if(buf[0] == 'q'){
				printf("%s\n", "Comando de finalización recibido, saliendo...\0");
				break;
			} else {
				printf("%s\n", "Comando no soportado...\0");
			}
		}
		
	}
	
	return 0;
}