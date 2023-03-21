#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>

#define ROSU  "\x1b[31m"
#define ALBASTRU "\x1b[36m"
#define GALBEN "\x1b[33m"
#define VERDE  "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define ROZ  "\x1b[91m"
#define RESET   "\x1b[0m"
#define bold "\x1b[1m"

extern int errno;

int port;
int nr=0;
  char buf[10];
int main (int argc, char *argv[])
{
    int sd,ok=1;
    struct sockaddr_in server;
    char msg[1001];
    if (argc != 3)
    {
        printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }
    port = atoi (argv[2]);
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror ("Eroare la socket().\n");
        return errno;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons (port);
    if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
        perror ("Eroare la connect().\n");
        return errno;
    }
  printf (bold ALBASTRU "---------Bine ati venit!------\n");
  printf (bold MAGENTA "Comenzile disponibile sunt:\n");
  printf ("Inregistrare <userName> <parola>: creeaza un nou cont \n");
  printf ("Conectare <userName> <parola>: conecteaza-te la un cont existent \n");
  printf ("UseriExistenti: afiseaza userii existenti pana acum \n");
  printf ("<quit> :inchidere aplicatie\n");

    
    while(ok)
    { 
        bzero (msg, 1001);
        printf (bold MAGENTA"Introduceti o comanda: \n" RESET);
        fflush (stdout);
        read (0, msg, 1001);
         if(strstr(msg,"quit"))
        {
          //printf("INTRA AICI \n");
          ok=0; close(sd);exit(0);
        }
        if (write (sd,msg,1001) <= 0)
         {
          perror ("[client]Eroare la write() spre server.\n");
          return errno;
        }
        /* citirea raspunsului dat de server 
      (apel blocant pina cind serverul raspunde) */
      bzero(msg,1001);
        if (read (sd, msg,1001) < 0)
     {
          perror ("[client]Eroare la read() de la server.\n");
          return errno;
     }
     printf(bold GALBEN"Mesajul primit %s\n" RESET,msg);
        if(ok==0){close(sd);exit(0);}
       }
    
    close (sd);
}