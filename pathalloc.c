/* Define for our path_alloc function that we are using from online                                   
source code                                                                                           
*/

#ifdef PATH_MAX
static long pathmax = PATH_MAX;
#else
static long pathmax = 0;
#endif

static long posix_version = 0;
static long xsi_version = 0;

/* If PATH_MAX is indeterminate, no guarantee this is adequate */

#define PATH_MAX_GUESS 1024

// Our path_alloc function we received from online source code                                        

char * path_alloc(size_t *sizep) {

  char *ptr;
  size_t size;

  if (posix_version == 0)
    posix_version = sysconf(_SC_VERSION);

  if (xsi_version == 0)
    xsi_version = sysconf(_SC_XOPEN_VERSION);

  if (pathmax == 0) {/* first time through */
    errno = 0;
    if ((pathmax = pathconf("/", _PC_PATH_MAX)) < 0) {
      if (errno == 0)
        pathmax = PATH_MAX_GUESS;/* it's indeterminate */
      else
        err_sys("pathconf error for _PC_PATH_MAX");
    } else {
      pathmax++;/* add one since it's relative to root */
    }
  }

  if ((ptr = malloc(size)) == NULL)
    err_sys("malloc error for pathname");

  if (sizep != NULL)
    *sizep = size;
  return(ptr);
}
