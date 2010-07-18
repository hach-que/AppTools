// main.h : Predefines all of the functions so that they may reference each other.
//

#include <iostream>
#include <strstream>
#include <string>
#include <vector>
#include <map>
#include <fs.h>

typedef void (*commandFunc)(std::vector<std::string>);
extern std::string openPackage;
extern std::map<std::string, std::string> environmentMappings;
extern std::map<std::string, commandFunc> availableCommands;
extern std::vector<std::string> commandHistory;
extern bool usedHistory;
extern FS * currentFilesystem;
extern std::fstream * fd;
extern std::string currentPath;
std::vector<std::string> parseCommand(std::string cmd);
void showHeaders();
void loadCommands();
void loadStartupFile();
void runLoop();
std::string readLine();
std::string readLineFromFile(std::fstream * data);
bool packageIsOpen();
bool checkPackageOpen();
void showCurrentOpenPackage();
bool checkArguments(std::string name, std::vector<std::string> args, int minargs, int maxargs = -1);
void doOpen(std::vector<std::string> cmd);
void doCreate(std::vector<std::string> cmd);
void doCd(std::vector<std::string> cmd);
void doLs(std::vector<std::string> cmd);
void doRm(std::vector<std::string> cmd);
void doTruncate(std::vector<std::string> cmd);
void doMkdir(std::vector<std::string> cmd);
void doTouch(std::vector<std::string> cmd);
void doEdit(std::vector<std::string> cmd);
void doClose(std::vector<std::string> cmd);
void doEnvSet(std::vector<std::string> cmd);
void doEnvGet(std::vector<std::string> cmd);
int main(int argc, char* argv[]);

// Utility functions
bool fileExists(std::string filename);
void printListHeaders(uint16_t id, uint16_t total);
void getFormattedPermissionBlock(uint8_t m, char * out);
void getFormattedPermissionMask(uint16_t mask, INodeType type, char * out);
void formatDateTime(uint32_t time, char * out);
void printListEntry(uint16_t id, uint16_t mask, INodeType type, uint16_t uid, uint16_t gid, uint32_t size, unsigned long mtime, char * filename);
std::string combinePaths(std::string base, std::string path);
std::string sanitizeAbsolutePath(std::string path);