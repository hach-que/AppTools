/*

Code file for main.cpp

This file is the code file for AppUtil.

This software is licensed under an MIT license.  See
http://code.google.com/p/apptools-dist for more information.

*/

#include <Python.h>
#include <libapp/logging.h>
#include <libapp/util.h>
#include <argtable2.h>
#include <iostream>
#include <string>
#include <cstdio>
#include <libgen.h>

std::string getInit()
{
	char* path = AppLib::LowLevel::Util::getProcessFilename();
	char* opath = path;
	if (path == NULL)
		return "";
	char* alt = dirname(path);
	if (alt != NULL && alt != opath)
		path = alt;
	std::string cmd = "";
	cmd += "import sys\n";
	cmd += "sys.path.insert(0, \"" + std::string(path) + "\")\n";
	cmd += "del sys\n";
	if (alt != NULL && alt != opath)
		free(opath);
	return cmd;
}

int runPython(const char* path, int argc, char* argv[])
{
	Py_Initialize();
	PyRun_SimpleString(getInit().c_str());
	FILE* fp = fopen(path, "r");
	PyRun_SimpleFile(fp, path);
	fclose(fp);
	Py_Finalize();

	return 0;
}

int main(int argc, char* argv[])
{
	// Setup.
	AppLib::Logging::setApplicationName(std::string("apputil"));
	struct arg_file *script = arg_file1(NULL, NULL, "script", "the path of the script to execute");
	struct arg_end *end = arg_end(20);
	void *argtable[] = { script, end };
	script->filename[0] = NULL;

	// Check to see if the argument definitions were allocated
	// correctly.
	if (arg_nullcheck(argtable))
	{
		AppLib::Logging::showErrorW("Insufficient memory.");
		return 1;
	}

	// Now parse the arguments.
	int nerrors = arg_parse(argc, argv, argtable);

	// Check to see if there were errors.
	if (nerrors > 0 && script->filename[0] == NULL)
	{
		printf("Usage: apputil");
		arg_print_syntax(stdout, argtable, "\n");
		arg_print_errors(stdout, end, "apputil");
		
		printf("AppUtil - The application scripting utility.\n\n");
		arg_print_glossary(stdout, argtable, "    %-25s %s\n");
		return 1;
	}

	// Execute.
	return runPython(script->filename[0], argc, argv);
}

