#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h> 		// descripteurs de fichiers ( open()...)
#include <termios.h>	// 
#include <errno.h>
#include <iostream>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

/* baudrate settings are defined in <asm/termbits.h>, which is included by <termios.h> */
#define BAUDRATE B115200            
/* change this definition for the correct port */
#define MODEMDEVICE "/dev/ttyS0"
#define _POSIX_SOURCE 1 /* POSIX compliant source */

#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE; 
int main()
{
  	int sfd, c, res;
    struct termios newtio;
    char buf[255];
    ssize_t size;
	
	sfd = open(MODEMDEVICE, O_RDONLY | O_NOCTTY ); 
	if (sfd == -1)
	{
	  printf ("Error  no is : %d\n", errno);
	  printf("Error description is : %s\n",strerror(errno));
	  return(-1);
	}
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        /* 
          BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
          CRTSCTS : output hardware flow control (only used if the cable has
                    all necessary lines. See sect. 7 of Serial-HOWTO)
          CS8     : 8n1 (8bit,no parity,1 stopbit)
          CLOCAL  : local connection, no modem contol
          CREAD   : enable receiving characters
        */
    newtio.c_cflag = BAUDRATE | CRTSCTS | CS8 | CLOCAL | CREAD;
         
        /*
          IGNPAR  : ignore bytes with parity errors
          ICRNL   : map CR to NL (otherwise a CR input on the other computer
                    will not terminate input)
          otherwise make device raw (no other input processing)
        */
    newtio.c_iflag = IGNPAR | ICRNL;
         
       /*
          ICANON  : enable canonical input
          disable all echo functionality, and don't send signals to calling program
        */
    newtio.c_lflag = ICANON;
         

	/* 
	  now clean the modem line and activate the settings for the port
	*/
	 tcflush(sfd, TCIFLUSH);
	 tcsetattr(sfd,TCSANOW,&newtio);
	
	for (int i =0 ; i<50 ; i++)
	{
		
	/*
	  ** On lit :
	  ** on passe a read :
	  ** - le fd,
	  ** - le buffer
	  ** - la taille du buffer
	  ** Attention si tu passe une taille de buffer plus grande que la taille de ton buffer,
	  ** ton programme deviens sensible aux Buffer Overflow
	  */
	  size = read (sfd, buf, 127);
   
	  /*
	  ** On raoute un '\0' à la fin de la chaine lut, pour être sur d'avoir une chaine de caractères valide.
	  ** size correspondant a l'index du dernier caractere du buffer + 1.
	  ** Ceci est utile si tu veux utiliser ta chaine dans une fonction comme strcmp() ou printf()
	  */
 
	  buf[size] = 0;
   
	  /*
	  ** On affiche ce que l'on viens de lire dans la console :
	  ** NOTE :
	  ** il existe des FD speciaux :
	  ** Le fd 1 est la sortie standart ( console )
	  */
 
	  //write (1, buf, size);
	  std::cout << i << " " << buf << "\n";
	  }
 
  /* Ne pas oublier de libérer ton file descriptor */
  close(sfd);
  
  try {
  // Les variables nécessaires à notre programme
  sql::Driver* driver;
  sql::Connection* con;
  sql::Statement* stmt;
  sql::ResultSet* res;
 
  // Etape 1 : créer une connexion à la BDD
  driver = get_driver_instance();
  // on note les paramètres classiques: adresse ip du serveur et port, login, mot de passe
  con = driver->connect("localhost", "bts", "snir");
 
  // Etape 2 : connexion à la base choisie, ici olivier_db
  con->setSchema("capteurs");
 
  // Etape 3 : création d'un objet qui permet d'effectuer des requêtes sur la base
  stmt = con->createStatement();
 
  // Etape 4 : exécution d'une requete : ici on sélectionne tous les enregistrements
  // de la table Animal
  res = stmt->executeQuery("SELECT * FROM relever");
  
  // Etape 5 : exploitation du résultat de la requête
  while (res->next()) {
    std::cout << "\t... MySQL a repondu: ";
    // Acces par non du champ de la table : ici le champ 'id' que l'on recupère au format string
    std::cout << res->getString("id") << std::endl;
    std::cout << "\t... MySQL la suite : ";
    // Acces à la donnée par son numéro de colonne, 1 étant le premier (ici 'id'), 5 le nom de l'animal
    std::cout << res->getString(5) << std::endl;
  }
 
  // On nettoie tout avant de sortir : effacement des pointeurs
  // le pointeur sur le Driver sera effacé tout seul
  delete res;
  delete stmt;
  delete con;
 
} catch (sql::SQLException &e) {
  // Gestion des execeptions pour déboggage
  std::cout << "# ERR: " << e.what();
  std::cout << " (code erreur MySQL: " << e.getErrorCode();
  std::cout << ", EtatSQL: " << e.getSQLState() << " )" << std::endl;
}
 
  std::cout << std::endl;
 
  return 0;
}
