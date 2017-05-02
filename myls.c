#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include "myls.h"

#define KB 1024
#define MB 1048576
#define GB 1073741824

int c_flag = 0;
int d_flag = 0;
int l_flag = 0;
int f_flag = 0;
int h_flag = 0;
int r_flag = 0;

struct stat metadata;
int numOptions = 0;
unsigned long long blockSize;


int main(int argc, char *argv[])
{
  parseCommandLine(argc, argv);
  
  if (optind == argc) // no argument after options
    simpleLs(".");
  else
    simpleLs(argv[argc-1]);
}


void parseCommandLine(int argc, char *argv[])
{
  int option;
  while ((option = getopt(argc, argv, "cd:lfhr")) != -1) {
    switch (option) {
    case 'c':
      c_flag = 1;
      break;
    case 'd':
      blockSize = atol(optarg); // Add error for no arg ???????????????????????????????
      printf("%llu\n", blockSize);
      d_flag = 1;
      break;
    case 'l':
      l_flag = 1;
      break;
    case 'f':
      f_flag = 1;
      break;
    case 'h':
      h_flag = 1;
      break;
    case 'r':
      r_flag = 1;
      break;
    default: /* '?' */
      fprintf(stderr, "Usage: %s [-c] [-d blocksize] [-l]  [-f] [-h] [-r] filename\n",
	      argv[0]);
      exit(EXIT_FAILURE);
    }
  }
}


void simpleLs(char *pathname)
{  
  int fileDescriptor = open(pathname,O_RDONLY);
  DIR *directory = opendir(pathname);

  if(directory != NULL)  // path refers to a directory
    {
      printf("This is a directory\n");
      traverseDirectory(directory);
    }
  else
    {
      if(fileDescriptor != -1)  // path refers to a file
	{
	  //printf("This is a file\n");	 
	  listfileInfo(fileDescriptor, pathname);
	}   
      else
	{
	  fprintf(stderr, "ls: cannot access '%s': No such file or directory\n", pathname);
	  //	  exit(EXIT_FAILURE);
	}
    }
}


void traverseDirectory(DIR *directory)
{
  //list of directories
  struct dirent *directEntry;
  while ((directEntry = readdir(directory)) != NULL)
    {     
      if( (strcmp(directEntry->d_name, "..") != 0) && (strcmp(directEntry->d_name, ".") != 0))
	{
	  simpleLs(directEntry->d_name);
	}
      /* if (directEntry->d_type == DT_DIR)
	 ;
	 else
	 simpleLs(directEntry->d_name);
      */
      //printf("%d %s\n", (int) directEntry->d_ino, directEntry->d_name);
    }
  closedir(directory);
}


void listfileInfo(int fileDescriptor, char *pathname)
{
  if(lstat(pathname, &metadata) == -1)
    {
      perror("stat");
      exit(EXIT_FAILURE);
    }
  
  if(l_flag)
    printLongListing();

  printFileName(pathname);
}


char checkForFileType()
{
  if (metadata.st_mode & S_IXUSR) // executable
    {
      return '*';
    }
  
  switch (metadata.st_mode & S_IFMT)
    {
    case S_IFLNK:     // symbolic link
      return '@';
      break;
    case S_IFDIR:     // directory
      return '/';
      break;
    default:
      return'\0';
    }
}
 

void printLongListing()
{
  struct passwd *pwd;
  struct group *grp;
  unsigned long long file_size;

  printf("%u ", (int) metadata.st_ino); // inode number
  
  printPermissions(); // mode

  if ((pwd = getpwuid(metadata.st_uid)) != NULL)
    printf("%s ", pwd->pw_name);   // owner
  
  if ((grp = getgrgid(metadata.st_gid)) != NULL)
    printf("%s ", grp->gr_name);   // group

  if(d_flag)
    file_size = calculateDiskFileSize(); 
  else
    file_size = metadata.st_size;

  if(h_flag)
    printHumanBytes(file_size);
  else
    printf("%lli  ", file_size); // file size
    
  printFileModTime();
}


void printPermissions()
{
  // File Type
  switch (metadata.st_mode & S_IFMT)
    {
    case S_IFLNK:     // symbolic link
      printf("l");
      break;
    case S_IFDIR:     // directory
      printf("d");
      break;
    case S_IFBLK:     // block special
      printf("b");
      break;
    case S_IFIFO:     // FIFO special
      printf("p");
      break;
    case S_IFSOCK:    // socket
      printf("s");
      break;
    case S_IFCHR:     // character special
      printf("c");
      break;
    default:
      printf("-");
    }

  // More permission options????????????????????????????????????????????
  // User Permissions
  (metadata.st_mode & S_IRUSR) ? printf("r") : printf("-");
  (metadata.st_mode & S_IWUSR) ? printf("w") : printf("-");
  (metadata.st_mode & S_IXUSR) ? printf("x") : printf("-");

  // Group Permissions
  (metadata.st_mode & S_IRGRP) ? printf("r") : printf("-");
  (metadata.st_mode & S_IWGRP) ? printf("w") : printf("-");
  (metadata.st_mode & S_IXGRP) ? printf("x") : printf("-");

  // Other Permissions
  (metadata.st_mode & S_IROTH) ? printf("r") : printf("-");
  (metadata.st_mode & S_IWOTH) ? printf("w") : printf("-");
  (metadata.st_mode & S_IXOTH) ? printf("x") : printf("-");
  printf(" ");
}


long calculateDiskFileSize()
{
  unsigned long long numBlocks;
  numBlocks = metadata.st_size/blockSize;
  if (metadata.st_size % blockSize != 0)
    numBlocks++;
  return numBlocks*blockSize;
}


void printHumanBytes(unsigned long long file_size)
{
  if (file_size > GB)
    printf("%.1fGB ", ((float) file_size)/GB);
  else if (file_size > MB)
    printf("%.1fMB ", ((float) file_size)/MB);
  else if (file_size > KB)
    printf("%.1fKB ", ((float) file_size)/KB);
  else
    printf("%lli  ", file_size);
}


void printFileModTime()
{
  int success;
  char timeBuff[14];
  time_t current = time (NULL);
  
  struct tm *temp;

  // Extract tm struct from metadata
  temp = localtime(&metadata.st_mtime);
  if (temp == NULL) perror("localtime");  
  struct tm fileBuff = *temp;

  // Extract tm struct from current time
  temp = localtime(&current);
  if (temp == NULL) perror("localtime");  
  struct tm currentBuff = *temp;

  
  if (currentBuff.tm_year == fileBuff.tm_year) // modified in current year
    {      
      success = strftime(timeBuff, sizeof timeBuff, "%h %d %H:%M", &fileBuff);
      if(!success) perror("strftime");
      printf("%s ", timeBuff);
    }
  else //modified in a previous year
    {
      success = strftime(timeBuff, sizeof timeBuff, "%h %d %Y", &fileBuff);
      if(!success) perror("strftime");
      printf("%s ", timeBuff);
    }
}


void printFileName(char *pathname)
{  
  // Traverse pathname until you reach filename  
  char* currToken, *prevToken;
  
  currToken = strtok(pathname, "/");
  while(currToken != NULL)
    {
      prevToken = currToken;
      currToken = strtok(NULL, "/");
    }

  // prevToken now equals filename
  if (c_flag)
    {
      char type = checkForFileType();
      printf("%s%c\n", prevToken, type);
    }
  else
    printf("%s\n", prevToken);
}
