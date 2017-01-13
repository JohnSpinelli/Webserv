/*Clayton Ezzell
  Group: Mike Haley, John Spinelli
  Webserver Code
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include "sys/types.h"
#include "sys/socket.h"
#include "netinet/in.h"
#include "sys/stat.h"

static void handleRequest(char *, int);
static void failPrint(char *, char *, char *, char *,char *, int);
static void successPrint(char *, char *, char *, char *, int);

int main (int argc, char *argv[])
{
  int portnum;
  int socketfd;
  int socketOption;
  int bindret;
  int listenret;
  int clientfd;
  socklen_t len;
  int processID;
  char sockbuff[1024];
  
  struct sockaddr_in serv_addr;
  struct sockaddr_in cli_addr;

  if (argc != 2)
    {
      //Checks for improper call to webserver
      perror("Incorrect call to webserver.");
      exit(1);
    }
  
  portnum = atoi(argv[1]);

  if((socketfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror("Socket failed.");
      exit(1);
    }
  
  socketOption = 1;
  if((setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR,(void *) &socketOption, sizeof(socketOption))) == -1)
    {
      perror("Failed to set options on socket.");
      exit(1);
    }
    
  /*http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/server.c*/
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portnum);
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    

  if((bindret = bind(socketfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr))) == -1)
    {
      perror("Bind Error");
      exit(1);
    }
    
  if ((listenret = listen(socketfd, 3)) == -1)
    {
      perror("Listen Error");
      exit(1);
    }

  while(1)
    {
      len = sizeof(cli_addr);
      if((clientfd = accept(socketfd, (struct sockaddr *) &cli_addr, &len)) == -1)
	{
	  perror("Client Accept Error");
	  exit(1);
	}

      if((processID = fork()) == 0)
	{
	  close(socketfd);
	  if(read(clientfd, sockbuff, 1024) == -1)
	    {
	      perror("Read Error");
	      exit(1);
	    }

	  handleRequest(sockbuff, clientfd);
	  exit(0);
	}
      
      close(clientfd);
    }
}

void handleRequest(char *sockbuff, int clientfd)
{
  char request[1024];
  char *version;
  char *requestform;
  char *tokens[3];
  char *split;
  char *tok = strtok(sockbuff, " \n");
  int i = 0;
  int statret;
  struct stat filestat;

  while(tok != NULL)
    {
      tokens[i] = tok;
      tok = strtok(NULL, " \n");
      i++;
    }
  
  requestform = tokens[0];
  request[0] = '.';
  request[1] = '\0';
  strcat(request, tokens[1]);
  version = tokens[2];

  if(strchr(request, '?') != NULL)
    {
      int tokencount;
      char *dynamic[] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL};
      char *tok2 = strtok(request, "?=&");
      while(tok2 != NULL)
	{
	  dynamic[tokencount] = tok2;
	  tok2 = strtok(NULL, "?=&");
	  tok2 = strtok(NULL, "?=&");
	  tokencount++;
	}
      
      if(strcmp(dynamic[0], "./my-histogram") == 0)
	{
	  if(tokencount <= 2 || tokencount > 7)
	    {
	      failPrint("400", "Bad Request", version, "text/plain", "400: Bad Request", clientfd);
	      exit(1);
	    }
	    
	  if((statret = stat(dynamic[1], &filestat)) == -1)
	    {
	      failPrint("404", "Not Found", version, "text/plain", "404: Not Found", clientfd);
	    }
	    
	  pid_t worker;
	  int status;
	  int waitret;
	  if ((worker = fork()) == 0)
	    {
	      execvp(dynamic[0], dynamic);
	      perror("Failed to exec");
	      exit(1);
	    }
	  waitpid(worker, &status, 0);
	    
	  if((worker = fork()) == 0)
	    {
	      char *argv2[] = {"./plot.cgi", NULL};
	      execvp(argv2[0], argv2);
	      perror("Failed to exec");
	      exit(1);
	    }
	  waitpid(worker, &status, 0);
	    
	  successPrint("200", "OK", version, "image/jpeg", clientfd);
	    
	  dup2(clientfd, 1);
	    
	  char *argv2[] = {"cat", "my_graph.jpeg", NULL};
	  execvp(argv2[0], argv2);
	  perror("Failed to exec");
	  exit(1);
	}

      if (strcmp(requestform, "GET") == 0)
	{
	  successPrint("200", "OK", version, "text/plain", clientfd);
	    
	  dup2(clientfd, 1);
	  close(clientfd);
	    
	  execvp(dynamic[0], dynamic);
	  perror("Failed to exec");
	  exit(1);
	}

      failPrint("501", "Not Implimented", version, "text/plain", "501: Not Implimented", clientfd);
      exit(1);
    }



  
  if (strcmp(requestform, "GET") != 0)
    {
      failPrint("501", "Not Implimented", version, "text/plain", "501: Not Implimented", clientfd);
      exit(1);
    }

  if((statret = stat(request, &filestat)) == -1)
    {
      failPrint("404", "Not Found", version, "text/plain", "404: Not Found", clientfd);
      exit(1);
    }

  if(S_ISDIR(filestat.st_mode))
    {
      successPrint("200", "OK", version, "text/plain", clientfd);

      dup2(clientfd, 1);
      close(clientfd);
      
      char *argv2[] = {"ls", request, NULL};
      execvp("ls", argv2);
      perror("Failed to exec.");
      exit(1);
    }      

  split = strrchr(request, '.');

  if(split != NULL)
    {
      if(strcmp(".jpg", split) == 0)
	{
	  successPrint("200", "OK", version, "image/jpeg", clientfd);
      
	  dup2(clientfd, 1);
	  close(clientfd);
      
	  char *argv2[] = {"cat", request, NULL};
	  execvp("cat", argv2);
	  perror("Failed to exec.");
	  exit(1);
	}
      
      if(strcmp(".jpeg", split) == 0)
	{
	  successPrint("200", "OK", version, "image/jpeg", clientfd);

	  dup2(clientfd, 1);
	  close(clientfd);

	  char *argv2[] = {"cat", request, NULL};
	  execvp("cat", argv2);
	  perror("Failed to exec.");
	  exit(1);

	}
      if(strcmp(".gif", split) == 0)
        {
          successPrint("200", "OK", version, "image/gif", clientfd);

          dup2(clientfd, 1);
          close(clientfd);

          char *argv2[] = {"cat", request, NULL};
          execvp("cat", argv2);
          perror("Failed to exec.");
          exit(1);
        }
      if(strcmp(".html", split) == 0)
        {
          successPrint("200", "OK", version, "text/html", clientfd);

          dup2(clientfd, 1);
          close(clientfd);

          char *argv2[] = {"cat", request, NULL};
          execvp("cat", argv2);
          perror("Failed to exec.");
          exit(1);
	}
      if(strcmp(".cgi", split) == 0)
        {
          successPrint("200", "OK", version, "text/html", clientfd);

          dup2(clientfd, 1);
          close(clientfd);

          char *argv2[] = {request, NULL};
          execvp(request, argv2);
          perror("Failed to exec.");
          exit(1);
        }

      failPrint("501", "Not Implimented", version, "text/plain", "501: Not Implimented", clientfd);
      exit(1);

    }

}

void failPrint(char *statuscode, char *statusmess, char *version, char * addit ,char *message, int clientfd)
{
  char buff[1024];
  int length;
  length = sprintf(buff, "%s %s %s\n%s\n\n%s", version, statuscode, statusmess, addit, message);
  write(clientfd, buff, length);
}

void successPrint(char *statuscode, char *statusmess, char *version, char *addit,int clientfd)
{
  char buff[1024];
  int length;
  length = sprintf(buff, "%s %s %s\n%s\n\n", version, statuscode, statusmess, addit);
  write(clientfd, buff, length);
}
