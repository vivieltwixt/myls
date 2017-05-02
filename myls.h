#ifndef _MYLS_H_
#define _MYLS_H_

void simpleLs(char *pathname);
void parseCommandLine(int argc, char *argv[]);
int traverseDirectory(const char *fpath, const struct stat *sb,
		      int typeflag);
void listfileInfo(char *pathname);
char checkForFileType();
void printLongListing();
void printPermissions();
long calculateDiskFileSize();
void printHumanBytes(unsigned long long file_size);
void printFileModTime();
void printFileName(char *pathname);
#endif
