/* 
 *
 My-Histogram.c 
 Group: John Spinelli
 Clayton Ezzell
 Mike Haley
 *
 *
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdarg.h>
#include <ctype.h>
#include "apue.h"
#include "pathalloc.c"

#define FTW_F 1 /* File other than directory */
#define FTW_D 2 /* Directory */
#define FTW_DNR 3 /* Directory that can't be read */
#define FTW_NS 4 /* File that we can't stat */

static char *fullpath;
static size_t pathlen;
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;

typedef int Myfunc(const char *pathname, const struct stat *statptr, int type);
static Myfunc myfunc;
static int myftw(char *pathname, Myfunc *func);
static int dopath(Myfunc *func);


int main(int argc, char *argv[]) {

  int ret; 
  char *pathname = NULL;
  
  pathname = argv[1];
  
  if (pathname == NULL) {
    printf("Error, no argument specified");
    return 0;
  }

  ret = myftw(pathname, myfunc); /* does it all */

  ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
  
  FILE *fp;
  fp = fopen("fileCount.dat", "w");
  fprintf(fp,"reg\t %d\n",nreg);
  fprintf(fp,"dir\t %d\n",ndir);
  fprintf(fp,"blk\t %d\n",nblk);
  fprintf(fp,"chr\t %d\n",nchr);
  fprintf(fp,"fif\t %d\n",nfifo);
  fprintf(fp,"lnk\t %d\n",nslink);
  fprintf(fp,"soc\t %d\n",nsock);
  fclose(fp);


  if(ntot==0)
    ntot=1;
    
 
  printf("regular files = %7ld, %5.2f %%\n", nreg,
	 nreg*100.0/ntot);
 
  printf("directories = %7ld, %5.2f %%\n", ndir,
	 ndir*100.0/ntot);
  
  printf("block special = %7ld, %5.2f %%\n", nblk,
	 nblk*100.0/ntot);
 
  printf("char special = %7ld, %5.2f %%\n", nchr,
	 nchr*100.0/ntot);
  
  printf("FIFOs = %7ld, %5.2f %%\n", nfifo,
	 nfifo*100.0/ntot);
  
  printf("symbolic links = %7ld, %5.2f %%\n", nslink,
	 nslink*100.0/ntot);
  
  printf("sockets = %7ld, %5.2f %%\n", nsock,
	 nsock*100.0/ntot);
  
  
  exit(ret);  


}



static int myftw(char *pathname, Myfunc *func) {

  fullpath = path_alloc(&pathlen);
  
  if (pathlen <= strlen(pathname)) {
    pathlen = strlen(pathname) * 2;
    if ((fullpath = realloc(fullpath, pathlen)) == NULL)
      err_sys("realloc failed");
  }
  
  strncpy(fullpath, pathname, pathlen);
  fullpath[pathlen - 1] = 0;

  return (dopath(func));
}




static int dopath(Myfunc* func) {

  struct stat statbuf;
  struct dirent *dirp;
  DIR *dp;
  int ret;
  char *ptr;


  if (lstat(fullpath, &statbuf) < 0) /* stat error */
    return(func(fullpath, &statbuf, FTW_NS));
  if (S_ISDIR(statbuf.st_mode) == 0) /* not a directory */
    return(func(fullpath, &statbuf, FTW_F));

  //It's a Directory

  if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
    return(ret);

  ptr = fullpath + strlen(fullpath); // Point to end of fullpath
  *ptr++ = '/';
  *ptr = 0;

  if ((dp = opendir(fullpath)) == NULL) // Can't read directory
    return(func(fullpath, &statbuf, FTW_DNR));

  while ((dirp = readdir(dp)) != NULL) {

    if (strcmp(dirp->d_name, ".") == 0 ||
        strcmp(dirp->d_name, "..") == 0)
      continue; // ignore dot and dot-dot

    strcpy(ptr, dirp->d_name); // append name after slash

    if ((ret = dopath(func)) != 0) // Recursive
      break;
  }

  ptr[-1] = 0; /* erase everything from slash onwards */

  if (closedir(dp) < 0)
    err_ret("can't close directory %s", fullpath);

  return(ret);
}



static int
myfunc(const char *pathname, const struct stat *statptr, int type)
{
  switch (type) {
  case FTW_F:
    switch (statptr->st_mode & S_IFMT) {
    case S_IFREG: nreg++; break;
    case S_IFBLK: nblk++; break;
    case S_IFCHR: nchr++; break;
    case S_IFIFO: nfifo++; break;
    case S_IFLNK: nslink++; break;
    case S_IFSOCK: nsock++; break;
    case S_IFDIR:
      printf("for S_IFDIR for %s", pathname);
      /* directories should have type = FTW_D */ 
    }

    break;
  case FTW_D:
    ndir++;
    break;
  case FTW_DNR:
    printf("can't read directory %s", pathname);
    break;
  case FTW_NS:
    printf("stat error for %s", pathname);
    break;
  default:
    printf("unknown type %d for pathname %s", type, pathname);
  }
  return(0); 
}


/*
 * Functions Needed For Pathalloc 
 */


static void err_doit(int, int, const char *, va_list);

void
err_ret(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
}

void
err_sys(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  exit(1);
}

void
err_exit(int error, const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, error, fmt, ap);
  va_end(ap);
  exit(1);
}

void
err_dump(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(1, errno, fmt, ap);
  va_end(ap);
  abort();/* dump core and terminate */
  exit(1);/* shouldn't get here */
}

void
err_msg(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
}

void
err_quit(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  err_doit(0, 0, fmt, ap);
  va_end(ap);
  exit(1);
}

static void
err_doit(int errnoflag, int error, const char *fmt, va_list ap)
{
  char buf[MAXLINE];

  vsnprintf(buf, MAXLINE, fmt, ap);
  if (errnoflag)
    snprintf(buf+strlen(buf), MAXLINE-strlen(buf), ": %s",
	     strerror(error));
  strcat(buf, "\n");
  fflush(stdout);/* in case stdout and stderr are the same */
  fputs(buf, stderr);
  fflush(NULL);/* flushes all stdio output streams */
}
