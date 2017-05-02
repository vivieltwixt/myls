#ifndef _MYLS_H_
#define _MYLS_H_

void simpleLs(char *pathname);
void parseCommandLine(int argc, char *argv[]);
void traverseDirectory(DIR *directory);
void listfileInfo(int fileDescriptor, char *pathname);
char checkForFileType();
void printLongListing();
void printPermissions();
long calculateDiskFileSize();
void printHumanBytes(unsigned long long file_size);
void printFileModTime();
void printFileName(char *pathname);
#endif
