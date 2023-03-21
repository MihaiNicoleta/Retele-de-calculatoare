#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include<sqlite3.h>

#define ROSU  "\x1b[31m"
#define ALBASTRU "\x1b[36m"
#define GALBEN "\x1b[33m"
#define VERDE  "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define ROZ  "\x1b[91m"
#define RESET   "\x1b[0m"
#define bold "\x1b[1m"
/* portul folosit */
#define PORT 3336

/* codul de eroare returnat de anumite apeluri */
extern int errno;
char a[1001][1001];
static void *treat(void *); /* functia executata de fiecare thread ce realizeaza comunicarea cu clientii */
//void raspunde(void *);

typedef struct {
	pthread_t idThread; //id-ul thread-ului
	int thCount; //nr de conexiuni servite
}Thread;

Thread *threadsPool; //un array de structuri Thread

int sd; //descriptorul de socket de ascultare
int nthreads;//numarul de threaduri
pthread_mutex_t mlock=PTHREAD_MUTEX_INITIALIZER;              // variabila mutex ce va fi partajata de threaduri

 void raspunde(int cl,int idThread);
 char userName[1001],parola[1001];
 char msgRasp[1001];
 char *tables;
 sqlite3* BD;
 int bd,nr_mesaje;
 int iduri[1001];
 char *mesaj_er;
 extern int errno;
 char sql[1001];
 char str[1001];
 sqlite3_stmt *stmt;
void Baza_de_date()
 {
    bd=sqlite3_open("exemplu.db",&BD);
    if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
      tables = "CREATE TABLE Users(UserName TEXT, Parola TEXT, Status TEXT);";
      bd=sqlite3_exec(BD,tables,0,0,&mesaj_er);
 }
void Baza_de_date2()
{
    bd=sqlite3_open("exemplu.db",&BD);
    if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
    tables = "CREATE TABLE Mess(ID INTEGER,Mesaj TEXT, Destinatar TEXT, Expeditor TEXT, Vazut TEXT,Timp TEXT);";
    bd=sqlite3_exec(BD,tables,0,0,&mesaj_er);
}
void Baza_de_date3()
{
    bd=sqlite3_open("exemplu.db",&BD);
    if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
    tables = "CREATE TABLE Nr_mesaje(NR INTEGER,User TEXT);";// daca nr_mesaje creste in timp ce sunt online
                                                             //-->arat pe ecran ca am mesaj nou
    bd=sqlite3_exec(BD,tables,0,0,&mesaj_er);
}

void threadCreate(int i)
{
	void *treat(void *);
	
	pthread_create(&threadsPool[i].idThread,NULL,&treat,(void*)i);
	return; /* threadul principal returneaza */
}
int main (int argc, char *argv[])
{
  Baza_de_date();
  Baza_de_date2();
  Baza_de_date3();
  struct sockaddr_in server;	// structura folosita de server  	
  void threadCreate(int);

   if(argc<2)
   {
        fprintf(stderr,"Eroare: Primul argument este numarul de fire de executie...");
	      exit(1);
   }
   nthreads=atoi(argv[1]);
   if(nthreads <=0)
	{
        fprintf(stderr,"Eroare: Numar de fire invalid...");
	      exit(1);
	}
    threadsPool = calloc(sizeof(Thread),nthreads);

   /* crearea unui socket */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("[server]Eroare la socket().\n");
      return errno;
    }
  /* utilizarea optiunii SO_REUSEADDR */
  int on=1;
  setsockopt(sd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on));
  
  /* pregatirea structurilor de date */
  bzero (&server, sizeof (server));

  /* umplem structura folosita de server */
  /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;	
  /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
  /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
  
  /* atasam socketul */
  if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
      perror ("[server]Eroare la bind().\n");
      return errno;
    }

  /* punem serverul sa asculte daca vin clienti sa se conecteze */
  if (listen (sd, 2) == -1)
    {
      perror ("[server]Eroare la listen().\n");
      return errno;
    }

   printf("Nr threaduri %d \n", nthreads); fflush(stdout);
   int i;
   for(i=0; i<nthreads;i++) threadCreate(i);

  /* servim in mod concurent clientii...folosind thread-uri */
  for ( ; ; ) 
  {
	printf ("[server]Asteptam la portul %d...\n",PORT);
        pause();				
  }
}
				
void *treat(void * arg)
{		
		int client;
		struct sockaddr_in from; 
 	     bzero (&from, sizeof (from));
 		printf ("[thread]- %d - pornit...\n", (int) arg);fflush(stdout);

		while(1)
		{
			int length = sizeof (from);
			pthread_mutex_lock(&mlock);
			//printf("Thread %d trezit\n",(int)arg);
			if ( (client = accept (sd, (struct sockaddr *) &from, &length)) < 0)
				{
	 			 perror ("[thread]Eroare la accept().\n");	  			
				}
			pthread_mutex_unlock(&mlock);
			threadsPool[(int)arg].thCount++;
			raspunde(client,(int)arg); //procesarea cererii
			/* am terminat cu acest client, inchidem conexiunea */
			close (client);		
		}
		
}
static int callback (void *str, int argc, char **argv, char **azColName)
{
    int i;
    char* data = (char*) str;

    for (i = 0; i < argc; i++)
    {
        if (argv[i])
        {
             strcat (data, argv[i]);
            strcat (data, "\n");
        }
        else
            strcat (data, "NULL"); 
    }

    return 0;
}

void RaspundeMesaj(int cl,int idThread,char user3[1001],char zi[1001])
{
     //RaspundeMesaj <user2>
     bd=sqlite3_open("exemplu.db",&BD);
     if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
    char raspuns5[1001];
    bzero(raspuns5,1001);
    //user-ul 1 este a[idThread]
    char mesaj2[1001];
    bzero(mesaj2,1001);
    //citim mesaj la care vrem sa raspundem data+ora
    //AAAA-MM-ZZ HH:MM:SS 
    // citim raspunsul la mesaj
    if (read (cl, mesaj2,sizeof(mesaj2)) <= 0)
	  {
		printf("[Thread %d]\n",idThread);
		perror ("Eroare la read() de la client.\n");
  	}else printf ("[Thread %d]Mesajul a fost receptionat...%s\n",idThread, raspuns5);
    //adaugam in mesaje mesaj2 cu insert in table
    //raspuns la mesajul de la data <zi>: <TrimiteMesaj>
    sprintf(raspuns5,"Raspunsul la mesajul de la data de %s catre %s este %s\n",zi,user3,mesaj2);
    if (write (cl, raspuns5, 1001) <= 0)
		{
		 printf("[Thread %d] ",idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
   else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);	 
    sql[0] = 0;
    str[0] = 0;
    char sir[101]; strcpy(sir,"nevazut");
    char ceva1[1001];bzero(ceva1,1001);
    sprintf(ceva1,"SELECT datetime('now');");
    bd = sqlite3_exec ( BD, ceva1, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }   
    printf("CEVA 1 %s\n",str);
    strcpy(ceva1,str);
    char k[1001]; bzero(k,1001);
    sprintf(k,"^Raspuns la mesajul de la data %s:^ ",zi);
    sql[0] = 0;
    str[0] = 0;
    char seen[1001]; strcpy(seen,"nevazut");
    strcat(k,mesaj2);
    strcpy(mesaj2,k);
    sprintf(sql,"INSERT INTO Mess (ID,Mesaj,Destinatar,Expeditor,Vazut,Timp) VALUES ('%d','%s','%s','%s','%s','%s');",nr_mesaje,mesaj2,user3,a[idThread],sir,ceva1);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    } 
    // adaugam nr mesaje primite de destinatar pt ca a mai primit 1
    //dau select la ce nr avea inainte si apoi il cresc
    // daca nr a crescut si destinatarul este online va aparea
    //pe ecran HEI AI MESAJ NOU DE LA ...
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "SELECT NR FROM Nr_mesaje WHERE User='%s';",user3);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    int nr=atoi(str);
    printf("NR RECENT DE MESAJE %d\n",nr);
    //
    nr++;//am primit mesaj nou deci crestem numarul
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "UPDATE Nr_mesaje SET NR='%d' WHERE User='%s';",nr,user3);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }

}
void MesajeNecitite(int cl,int idThread)
{
  // vrem sa vedem cate mesaje necitite avem
  // de ex aveti 2 mesaje necitte de la mara
  // aveti 3 mesaje necitite de la nico
  //pentru a vedea mesajele necitite de la mara deschideti conversatia cu ea
  //cand deschidem conversatia acestea devin citite
    bd=sqlite3_open("exemplu.db",&BD);
     if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
    char raspuns4[1001];
    bzero(raspuns4,1001);
    //user-ul 1 este a[idThread]
    char user2[1001];
    bzero(user2,1001);
    //citim userul2 ->cel cu care vrem sa vedem cate mesaje necitite avem
    if (read (cl, user2,sizeof(user2)) <= 0)
	  {
		printf("[Thread %d]\n",idThread);
		perror ("Eroare la read() de la client.\n");
  	}else printf ("[Thread %d]Mesajul a fost receptionat...%s\n",idThread, user2);
    //trb sa vedem daca user2 exista
    sprintf(raspuns4,"Numarul de mesaje necitite primite de %s de la %s \n",a[idThread],user2);
    sql[0] = 0;
    str[0] = 0;
    user2[strlen(user2)-1]=NULL;
   // printf("DESTINATAR %s LUNGIME %d \n",user2,strlen(user2));
   // printf("EXPEDITOR %s LUNGIME %d \n",a[idThread],strlen(a[idThread]));
    sprintf(sql,"SELECT  COUNT(Vazut) FROM Mess WHERE Vazut='nevazut' AND Destinatar='%s' AND Expeditor='%s';" ,a[idThread],user2);
    bd= sqlite3_exec (BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp,"Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    // punem in raspuns conversatia
    printf("Mesaje Necitite  %s\n",str);
    strcat(raspuns4,str);
    //aratam si care sunt mesajele necitite si le facem citite
    sql[0] = 0;
    str[0] = 0;
    sprintf(sql,"SELECT Expeditor || ': ' || Mesaj, Timp FROM Mess WHERE Vazut='nevazut' AND Destinatar='%s' AND Expeditor='%s';" ,a[idThread],user2);
    bd= sqlite3_exec (BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp,"Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
      
      strcat(raspuns4,"\n");
      strcat(raspuns4,str);
    // le facem citite dupa ce le-am vazut
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "UPDATE Mess SET Vazut='citit' WHERE Destinatar = '%s' AND Expeditor='%s';",a[idThread],user2);
    bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
     }
    if (write (cl, raspuns4, 1001) <= 0)
		{
		 printf("[Thread %d] ",idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
  else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);	  
  
}
void Conversatie(int cl,int idThread)
{
   bd=sqlite3_open("exemplu.db",&BD);
  if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
    char raspuns4[1001];
    bzero(raspuns4,1001);
    //user-ul 1 este a[idThread]
    char user2[1001];
    bzero(user2,1001);
    //citim userul2
    if (read (cl, user2,sizeof(user2)) <= 0)
	  {
		printf("[Thread %d]\n",idThread);
		perror ("Eroare la read() de la client.\n");
  	}else printf ("[Thread %d]Mesajul a fost receptionat...%s\n",idThread, user2);
    //trb sa vedem daca user2 exista
     user2[strlen(user2)-1]=NULL;
    sprintf(raspuns4,"Deschidem conversatia dintre %s si %s \n",a[idThread],user2);
      // toate mesajele nevazute de la user2 devin vazute    
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "UPDATE Mess SET Vazut='citit' WHERE Destinatar = '%s' AND Expeditor='%s';",a[idThread],user2);
    bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
     }  
    sql[0] = 0;
    str[0] = 0;
   // printf("DESTINATAR %s LUNGIME %d \n",user2,strlen(user2));
   // printf("EXPEDITOR %s LUNGIME %d \n",a[idThread],strlen(a[idThread]));
    sprintf(sql,"SELECT Expeditor || ':' || Mesaj || '<' || Timp || '>' || '*' || Vazut || '*' FROM Mess WHERE Expeditor IN ('%s','%s') AND Destinatar IN ('%s','%s') ORDER BY Timp;",user2,a[idThread],a[idThread],user2);
   
    bd= sqlite3_exec (BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp,"Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    // punem in raspuns conversatia
    printf("CONVERSATIE %s\n",str);
    strcat(raspuns4,str);
    if (write (cl, raspuns4, 1001) <= 0)
		{
		 printf("[Thread %d] ",idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
  else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);	 
}
void TrimiteMesaj(int cl,int idThread, char userName[1001],char parola[1001])
{
  //mesajul trimis ramane necitit pana cand expeditorul deschide acea conversatie
  bd=sqlite3_open("exemplu.db",&BD);
  if(bd!=SQLITE_OK)
    printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
  char raspuns3[1001];
  bzero(raspuns3,1001);
  //citim destinatarul mesajului si mesajul
  //userName-ul nostru este expeditorul
  char mesaj[1001];
  bzero(mesaj,1001);
  if (read (cl, mesaj,sizeof(mesaj)) <= 0)
	{
		printf("[Thread %d]\n",idThread);
		perror ("Eroare la read() de la client.\n");
	}
	  printf ("[Thread %d]Mesajul a fost receptionat...%s\n",idThread, mesaj);
    //pana la primul spatiu e numele destinatarului
    char destinatar[1001];
    bzero(destinatar,1001);
    strcpy(destinatar,mesaj);
    int i=0;
    while(destinatar[i]!=' ')
           i++;   
    strcpy(mesaj,destinatar+i+1);
    destinatar[i]=0;
    //daca user-ul caruia i-am trimis mesajul este inactiv->mesaj nevazut automat?  
    nr_mesaje++;
    // punem mesajul in baza de date
    // destinatarul= cel pe care l-am citit
    //expeditor=userul cu care am apelat a[idThread]
    sql[0] = 0;
    str[0] = 0;
    char sir[101]; strcpy(sir,"nevazut");
    char ceva[1001];
    sprintf(ceva,"SELECT datetime('now');");
    bd = sqlite3_exec ( BD, ceva, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }   
     printf("DATA %s \n",str);
     strcpy(ceva,str);
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "INSERT INTO Mess (ID,Mesaj, Destinatar , Expeditor, Vazut, Timp) VALUES ('%d' ,'%s', '%s','%s','%s','%s');",nr_mesaje,mesaj,destinatar,a[idThread],sir,ceva);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    // adaugam nr mesaje primite de destinatar pt ca a mai primit 1
    //dau select la ce nr avea inainte si apoi il cresc
    // daca nr a crescut si destinatarul este online va aparea
    //pe ecran HEI AI MESAJ NOU DE LA ...
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "SELECT NR FROM Nr_mesaje WHERE User='%s';",destinatar);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    int nr=atoi(str);
    printf("NR RECENT DE MESAJE %d\n",nr);
    //
    nr++;//am primit mesaj nou deci crestem numarul
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "UPDATE Nr_mesaje SET NR='%d' WHERE User='%s';",nr,destinatar);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    // daca destinatar-ul este online il notificam sa deschisa
    // conversatia din nou pt care are un nou mesaj
    sql[0] = 0;
    str[0] = 0;
    sprintf (sql, "SELECT Status FROM Users WHERE UserName='%s';",destinatar);
    bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
    if (bd != SQLITE_OK)
    {
        sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
        sqlite3_free (mesaj_er);
    }
    sprintf (raspuns3,"Destinatarul este: %s.\n Mesajul trimis este: %s \n Expeditorul este: %s\n",destinatar,mesaj,a[idThread]);
    str[strlen(str)-1]=0;
    printf("STATUSUL %s lungime %d\n",str,strlen(str));
    if(strcmp(str,"activ")==0)
    {
      char reply[1001]; bzero(reply,1001);
        sprintf(reply,"\nUser-ul: %s are un mesaj nou\nDeschideti conversatia pentru vizualizare",destinatar);
     strcat(raspuns3,reply);
    }
    printf("RASPUNS 3 %s\n",raspuns3);
     if (write (cl, raspuns3, 1001) <= 0)
		{
		 printf("[Thread %d] ",idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
  else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);	 

    //
}
void meniu(int cl,int idThread, char userName[1001],char parola[1001])
{
  char raspuns2[1001];
  bzero(raspuns2,1001);
  int conectat=1;
  while(conectat)
    {
      char mesaj[1001];
      bzero(mesaj,1001);
      if (read (cl, mesaj,sizeof(mesaj)) <= 0)
			{
        close(cl); break;
			  printf("[Thread %d]\n",idThread);
			  perror ("Eroare la read() de la client.\n");
			}
	      printf ("[Thread %d]Mesajul a fost receptionat...%s\n",idThread, mesaj);
		    char copie[1001];
        strcpy(copie,mesaj);
        bzero(raspuns2,1001);
        if(strstr(copie,"StatusUseri")!=0)
        {
          sql[0] = 0;
          str[0] = 0;
          sprintf (sql, "SELECT UserName ||' este '|| Status FROM Users;");
          bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
          if (bd != SQLITE_OK)
          {
             sprintf (raspuns2, "Eroare SQLITE %s\n", mesaj_er);
              sqlite3_free (mesaj_er);
          }
          else 
            sprintf (raspuns2,"Userii sunt %s\n", str);

        }else  if(strstr(copie,"Deconectare")!=0) 
        { 
          
          conectat=0;
          //daca scriem deconectare deconectam user-ul cu care suntem acum online
            sql[0] = 0;
            str[0] = 0;
            sprintf (sql, "UPDATE Users SET Status='inactiv' WHERE UserName = '%s';", a[idThread]);
             bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
              if (bd != SQLITE_OK)
               {
                  sprintf (raspuns2, "Eroare SQLITE %s\n", mesaj_er);
                  sqlite3_free (mesaj_er);
               } else 
            bzero(raspuns2,1001);
            sprintf (raspuns2,"Userul %s a fost deconectat \n",a[idThread]);
            strcat (raspuns2,"Ati revenit in meniul principal \n");
            strcat (raspuns2,"Comenzi disponibile acum: \n");
            strcat (raspuns2,"Inregistrare <userName> <parola>: creeaza un nou cont \n");
            strcat (raspuns2,"Conectare <userName> <parola>: conecteaza-te la un cont existent \n");
          
       }else  if(strstr(copie,"TrimiteMesaj")!=0) 
       {
            bzero(raspuns2,1001);
            sprintf (raspuns2,"Introduceti userul caruia vreti sa ii trimiteti un mesaj\n");
            strcat (raspuns2,"si mesajul dorit sub formatul <numeUser> <<mesaj>>\n");
       }else if(strstr(copie,"Conversatie")!=0) 
       {
            bzero(raspuns2,1001);
            sprintf (raspuns2,"Introduceti numele userul cu care vreti sa deschideti conversatia:<UserName>\n");
       }else  if(strstr(copie,"MesajeNecitite")!=0) 
       {
          bzero(raspuns2,1001);
          sprintf (raspuns2,"Introduceti numele userul cu care vreti sa vedeti numarul de mesaje necitite:<UserName>\n");

       }else  if(strstr(copie,"RaspundeMesaj")!=0) 
       {
          bzero(raspuns2,1001);
          sprintf (raspuns2,"Introduceti raspunsul la mesaj\n");

       }
     if (write (cl, raspuns2, 1001) <= 0)
		{
		 printf("[Thread %d] ",idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
  else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);	   
    if(strstr(copie,"TrimiteMesaj")!=0) 
    {
        TrimiteMesaj(cl,idThread,userName,parola);
    }else  if(strstr(copie,"Conversatie")!=0) 
    {
        Conversatie(cl,idThread);
    }else  if(strstr(copie,"MesajeNecitite")!=0) 
    {
        MesajeNecitite(cl,idThread);
    }else if(strstr(copie,"RaspundeMesaj")!=0) 
    {
            printf("USER 2 %s\n",copie);

      //RaspundeMesaj <user2> iau user-ul sa-l dau ca argument
      char mesaj1[1001];
      bzero(mesaj1,1001);
      char user3[1001]; 
      bzero(user3,1001);
      int i=0;
      while(copie[i]!=' ')
                i++;
     strcpy(user3,copie+i+1);
      copie[i]=0;
      user3[strlen(user3)-1]=0;
      int j=0;
      while(user3[j]!=' ')
                j++;
      strcpy(mesaj1,user3+j+1);
      user3[j]=0;
      mesaj1[strlen(mesaj)]=0;
      //printf("USER 3 %s\n",user3);
     // printf("Lungime %d\n",strlen(user3));
      //printf("MESaj 3 %s\n",mesaj1);
     // printf("Lungime %d\n",strlen(mesaj1));
      RaspundeMesaj(cl,idThread,user3,mesaj1);
    }
  }
    
}
void raspunde(int cl,int idThread)
{
    int ok=1;
    char msg[1001];//mesajul primit de trimis la client  
    int bd=sqlite3_open("exemplu.db",&BD);
    if(bd!=SQLITE_OK)
      printf("Nu se poate deschide baza de date: %s /n",sqlite3_errmsg(BD));
	 while(ok)
    {
         bzero(msg,1001);
         if (read (cl, msg,sizeof(msg)) <= 0)
			{
        close(cl); break;
			  printf("[Thread %d]\n",idThread);
			  perror ("Eroare la read() de la client.\n");
			}
	      printf ("[Thread %d]Mesajul a fost receptionat...%s\n",idThread, msg);
		    char copie[1001];
        strcpy(copie,msg);
        bzero(msgRasp,1001);
       if(strstr(copie,"Inregistrare")!=0)
        {   
            // daca am primit inregistrare trb sa avem si username si parola
            bzero(msgRasp,1001);
            bzero(userName,1001);
            bzero(parola,1001);
            int i=0;
            while(copie[i]!=' ')
                i++;
            strcpy(userName,copie+i+1);
            copie[i]=0;
            userName[strlen(userName)-1]=0;
            int j=0;
            while(userName[j]!=' ')
                j++;
            strcpy(parola,userName+j+1);
            userName[j]=0;
            parola[strlen(parola)]=0;
            strcpy(a[idThread],userName);
            printf(bold MAGENTA"Am citit username-ul: %s \n" RESET,userName);
            printf(bold MAGENTA"Am citit parola: %s \n" RESET,parola);
           // verific sa nu mai exista un username la fel
             sql[0] = 0;
             str[0] = 0;
              sprintf (sql, "SELECT UserName FROM Users WHERE UserName = '%s';", userName);
               bd= sqlite3_exec (BD, sql, callback, str, &mesaj_er);
                    if (bd != SQLITE_OK)
                    {
                        sprintf (msgRasp,"Eroare la SQLITE %s\n", mesaj_er);
                        sqlite3_free (mesaj_er);
                    }
                    else if (strstr (str, userName))
                    {
                        strcpy (msgRasp,bold GALBEN"UserName-ul deja exista\n"RESET );
                    }
                    else
                    {
                        sprintf (sql, "INSERT INTO Users (UserName, Parola, Status) VALUES ('%s', '%s','inactiv');", userName,parola);
                        bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
                        if (bd != SQLITE_OK)
                        {
                         // printf("AICI--2");
                            sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
                            sqlite3_free (mesaj_er);
                        }
                        else
                        {
                            str[0] = 0;
                            sprintf (sql, "SELECT * FROM Users;");
                            bd= sqlite3_exec (BD, sql, callback, str, &mesaj_er);
                            if (bd != SQLITE_OK)
                            {
                              //printf("AICI--3");
                                sprintf (msgRasp, "Eroare la SQLITE %s\n" , mesaj_er);
                                sqlite3_free (mesaj_er);
                            }
                            else
                            {
                                //printf ("%s", str);
                                sprintf (msgRasp,bold GALBEN"Inregistrarea a fost facuta cu succes\n Pentru accesare contului folositi comanda <Conectare>\n"RESET );
                                // in momentul in care inregistrez un nou user 
                               //ii fac numarul de mesaje primite din tabelul Nr_mesaje=0
                                int numar=0;
                                sql[0] = 0;
                                str[0] = 0;
                                sprintf (sql, "INSERT INTO Nr_mesaje (NR,User) VALUES ('%d' ,'%s');",numar,userName);
                                bd = sqlite3_exec ( BD, sql, callback, str, &mesaj_er);
                                if (bd != SQLITE_OK)
                              {
                                sprintf (msgRasp, "Eroare la SQLITE %s\n", mesaj_er);
                                sqlite3_free (mesaj_er);
                              }
                            }
                        }
                    }
        } 
          else if(strstr(copie,"Conectare")!=0) 
        {
            // daca am primit Conectare trb sa avem si username si parola
            // daca statusul este deja activ nu mai trebuie sa ne conectam
            bzero(msgRasp,1001);
            bzero(userName,1001);
            bzero(parola,1001);
            int i=0;
            while(copie[i]!=' ')
                i++;
            strcpy(userName,copie+i+1);
            copie[i]=0;
            userName[strlen(userName)-1]=0;
            int j=0;
            while(userName[j]!=' ')
                j++;
            strcpy(parola,userName+j+1);
            userName[j]=0;
            parola[strlen(parola)]=0;
            strcpy(a[idThread],userName);
            printf(bold MAGENTA"Am citit username-ul: %s \n" RESET,userName);
            printf(bold MAGENTA"Am citit parola: %s \n" RESET,parola);
          // trebuie sa verificam daca user si parola sunt in tabel
          // si daca e user-ul, verificam daca parola e corecta
          sql[0] = 0;
          str[0] = 0;
          sprintf (sql, "SELECT UserName FROM Users WHERE UserName = '%s';", userName);
          bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
          if (bd != SQLITE_OK)
          {
             sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
              sqlite3_free (mesaj_er);
          }
           else if (strstr (str, userName))
          {
             //printf ("%s", str);
             //daca userul exista verificam sa nu fie conectat deja
                sql[0] = 0;
                str[0] = 0;
          sprintf (sql, "SELECT Status FROM Users WHERE UserName = '%s';", userName);
          bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
          if (bd != SQLITE_OK)
          {
             sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
              sqlite3_free (mesaj_er);
          }
          if(strcmp(str,"activ")==0)
          {
            sprintf (msgRasp,bold ALBASTRU "Userul <%s> este deja conectat.\n" RESET, userName);
          }
          else
          {
               
             // daca exista user-ul, verificam daca parola e buna
              sql[0] = 0;
              str[0] = 0;
               sprintf (sql, "SELECT Parola FROM Users WHERE UserName = '%s';", userName);
               bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
               if (bd != SQLITE_OK)
               {
                  sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
                  sqlite3_free (mesaj_er);
               }
               if (strstr (str, parola))
               {
                sprintf (msgRasp,bold GALBEN "#Conectarea userului <%s> a fost facuta cu succes.\n" RESET, userName);
                sql[0] = 0;
                str[0] = 0;
               sprintf (sql, "UPDATE Users SET Status='activ' WHERE UserName = '%s';", userName);
               bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
               if (bd != SQLITE_OK)
               {
                  sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
                  sqlite3_free (mesaj_er);
               }
               }
                else
                sprintf (msgRasp,bold ROZ "Parola gresita,incearca din nou\n" RESET);
          }
          }
            else strcpy (msgRasp,bold ROZ "Nu exista un user cu acest nume,\n incearca din nou sau inregistreaza-te\n" RESET);
        }else if(strstr(copie,"UseriExistenti")!=0) 
        {
              sql[0] = 0;
              str[0] = 0;
          sprintf (sql, "SELECT UserName FROM Users;");
          bd = sqlite3_exec (BD, sql, callback, str, &mesaj_er);
          if (bd != SQLITE_OK)
          {
             sprintf (msgRasp, "Eroare SQLITE %s\n", mesaj_er);
              sqlite3_free (mesaj_er);
          }
         sprintf (msgRasp,bold ROZ "Userii existenti sunt: %s\n" RESET,str);
        }else if(strstr(copie,"quit")!=0) { sprintf (msgRasp,bold ROZ "Ati inchis clientul\n" RESET);
                        }
    if(strstr(msgRasp,"#")!=NULL)
   {
      bzero(msgRasp,1001);
      sprintf(msgRasp,bold GALBEN"-----#Conectare cu succes-----\n" RESET);
      strcat(msgRasp,bold GALBEN"Acum ai acces la noi comenzi\n" RESET);
      strcat(msgRasp,bold GALBEN"Pentru a te intoarce la meniul principal foloseste comanda <Deconectare>\n" RESET);
      strcat(msgRasp,bold ALBASTRU "COMENZI:\n" RESET);
      strcat(msgRasp,bold GALBEN"Introduceti comanda <StatusUseri> pentru vizualizarea useri-lor activi\n" RESET);
      strcat (msgRasp,bold GALBEN "<Conversatie>:deschide conversatia dintre user-ul curent si al user-\n mesajele primite de user-ul curent de la user2 care erau necitite devin citite \n");
      strcat(msgRasp,bold GALBEN"Introduceti comanda <TrimiteMesaj> pentru a trimite un mesaj unui anumit user\n" RESET);
      strcat(msgRasp,bold GALBEN"Introduceti comanda <RaspundeMesaj <user> <AAAA-MM-ZZ HH:MM:SS> >pentru a raspunde la un anumit mesaj\n" RESET);
      strcat(msgRasp,bold GALBEN"Introduceti comanda <MesajeNecitite> pentru a vedea cate mesaje necitite are un user-ul curent cu ceilalti useri\n" RESET);
      strcat(msgRasp,bold GALBEN "Introduceti comanda <Deconectare> pentru a deconecta user-ul curent\n" RESET);
   }
    if (write (cl, msgRasp, 1001) <= 0)
		{
		 printf("[Thread %d] ",idThread);
		 perror ("[Thread]Eroare la write() catre client.\n");
		}
	    else
		printf ("[Thread %d]Mesajul a fost trasmis cu succes.\n",idThread);	   
      if(strstr(msgRasp,"#")!=NULL)
    {
      //daca ne am conectat cu succes apelam functia meniu
      meniu(cl,idThread,a[idThread],parola);
    }    
	    
    //daca ne-am conectat cu succes, alta comanda
    

    }
}
