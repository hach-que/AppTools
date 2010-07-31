#include "logging.h"
#include "fs.h"
#include "fuselink.h"
#include "fsfile.h"
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <iostream>

bool file_exists(std::string filename);
void printListHeaders(uint16_t id, uint16_t total);
void getFormattedPermissionBlock(uint8_t m, char * out);
void getFormattedPermissionMask(uint16_t mask, AppLib::LowLevel::INodeType::INodeType type, char * out);
void formatDateTime(uint32_t time, char * out);
void printListEntry(uint16_t id, uint16_t mask, AppLib::LowLevel::INodeType::INodeType type, uint16_t uid, uint16_t gid, uint32_t size, unsigned long mtime, char * filename);