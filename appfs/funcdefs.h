/*

Header file for config.h

This file contains all of the function definitions.

Last edited by: James Rhodes <jrhodes@roket-enterprises.com>,
                20th July 2010

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

int main(int argc, char *argv[]);

#ifdef APPMOUNT
int appmount_start(int argc, char *argv[]);
void appmount_continue();
#else
int appfs_start(int argc, char *argv[]);
void appfs_continue();
#endif
