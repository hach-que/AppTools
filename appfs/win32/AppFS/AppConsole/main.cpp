// main.cpp : Defines the entry point for the console application.
//

#include "main.h"
#include <conio.h>

std::string openPackage = "";
std::map<std::string, std::string> environmentMappings;
std::map<std::string, commandFunc> availableCommands;
std::vector<std::string> commandHistory;
bool usedHistory = false;
AppLib::LowLevel::FS * currentFilesystem = NULL;
AppLib::LowLevel::BlockStream * fd = NULL;
std::string currentPath = "/";

int main(int argc, char* argv[])
{
	loadCommands();
	showHeaders();
	environmentMappings["editor"] = "notepad.exe";
	if (fileExists("./startup.txt"))
		loadStartupFile();
	runLoop();
}

void showHeaders()
{
	std::cout << "AppConsole for Win32.  Allows you to inspect AppFS packages via" << std::endl;
	std::cout << "a command-line interface, where FUSE is not available." << std::endl;
	std::cout << "Licensed under an MIT license." << std::endl;
	std::cout << std::endl;
}

void loadCommands()
{
	availableCommands["open"] = &doOpen;
	availableCommands["create"] = &doCreate;
	availableCommands["cd"] = &doCd;
	availableCommands["ls"] = &doLs;
	availableCommands["rm"] = &doRm;
	availableCommands["mkdir"] = &doMkdir;
	availableCommands["touch"] = &doTouch;
	availableCommands["edit"] = &doEdit;
	availableCommands["close"] = &doClose;
	availableCommands["truncate"] = &doTruncate;
	availableCommands["envset"] = &doEnvSet;
	availableCommands["envget"] = &doEnvGet;
}

void loadStartupFile()
{
	bool shouldBreak = false;
	std::fstream * sfile = new std::fstream("./startup.txt");
	if (sfile->bad() || sfile->fail())
	{
		delete sfile;
		return;
	}

	while (!sfile->eof())
	{
		if (packageIsOpen())
			std::cout << openPackage << " [ " << currentPath << " ]# ";
		else
			std::cout << "<NONE> [ " << currentPath << " ]# ";
		usedHistory = false;
		std::string cmdstr = readLineFromFile(sfile);
		std::vector<std::string> command = parseCommand(cmdstr);
		while (command.size() != 0 && command[0] == "__continue")
		{
			std::cout << ".. ";
			cmdstr += readLineFromFile(sfile);
			command = parseCommand(cmdstr);
		}
		if (!usedHistory)
			commandHistory.insert(commandHistory.end(), cmdstr);
		if (command.size() == 0)
		{
			std::cout << "# Bad input." << std::endl;
			continue;
		}
		
		if (command[0] == "exit")
		{
			sfile->close();
			delete sfile;
			return;
		}

		for (std::map<std::string,commandFunc>::iterator iter = availableCommands.begin(); iter != availableCommands.end(); iter++)
		{
			if (command[0] == iter->first)
			{
				iter->second(command);
				goto aftercmd_startup;
			}
		}
		
		std::cout << "# Bad command." << std::endl;
aftercmd_startup:
		continue;
	}

	sfile->close();
	delete sfile;
}

void runLoop()
{
	bool shouldBreak = false;
	while (!shouldBreak)
	{
		if (packageIsOpen())
			std::cout << openPackage << " [ " << currentPath << " ]# ";
		else
			std::cout << "<NONE> [ " << currentPath << " ]# ";
		usedHistory = false;
		std::string cmdstr = readLine();
		std::vector<std::string> command = parseCommand(cmdstr);
		while (command.size() != 0 && command[0] == "__continue")
		{
			std::cout << ".. ";
			cmdstr += readLine();
			command = parseCommand(cmdstr);
		}
		if (!usedHistory)
			commandHistory.insert(commandHistory.end(), cmdstr);
		if (command.size() == 0)
		{
			std::cout << "# Bad input." << std::endl;
			continue;
		}
		
		if (command[0] == "exit")
			return;

		for (std::map<std::string,commandFunc>::iterator iter = availableCommands.begin(); iter != availableCommands.end(); iter++)
		{
			if (command[0] == iter->first)
			{
				iter->second(command);
				goto aftercmd_standard;
			}
		}
		
		std::cout << "# Bad command." << std::endl;
aftercmd_standard:
		continue;
	}
}

std::string readLine()
{
	char c = '\0';
	std::string ret = "";
	int currentCommandHistoryPosition = 0;
	do
	{
		c = _getch();
		if (c == 0 || c == -32)
		{
			// Special key code was pressed.
			c = _getch();
			if (c == 72 || c == 80)
			{
				if (currentCommandHistoryPosition != 0)
					commandHistory[currentCommandHistoryPosition - 1] = ret;

				for (int i = 0; i < ret.length(); i += 1)
					std::cout << "\b \b";

				if (c == 72 && currentCommandHistoryPosition < commandHistory.size())
					currentCommandHistoryPosition += 1;
				else if (c == 80 && currentCommandHistoryPosition > 0)
					currentCommandHistoryPosition -= 1;

				if (currentCommandHistoryPosition == 0)
				{
					ret = "";
					// no need to print anything here
				}
				else
				{
					ret = commandHistory[currentCommandHistoryPosition - 1];
					for (int i = 0; i < commandHistory[currentCommandHistoryPosition - 1].length(); i += 1)
						std::cout << commandHistory[currentCommandHistoryPosition - 1][i];
				}
			}
			continue;
		}
		if (c < 32 && c != '\b' && c != '\r')
		{
			continue;
		}
		switch (c)
		{
			case '\r':
				std::cout << std::endl;
				break;
			case '\b':
				if (ret.size() > 0)
				{
					ret = ret.substr(0, ret.length() - 1);
					std::cout << "\b \b";
				}
				break;
			default:
				std::cout << c;
				ret += c;
				break;
		}
	}
	while (c != '\r');
	usedHistory = (currentCommandHistoryPosition != 0);
	if (currentCommandHistoryPosition != 0)
		commandHistory[currentCommandHistoryPosition - 1] = ret;
	return ret;
}

std::string readLineFromFile(std::fstream * data)
{
	char c = '\0';
	std::string ret = "";
	int currentCommandHistoryPosition = 0;
	do
	{
		data->get(c);
		if (c == 0 || data->eof())
		{
			std::cout << std::endl;
			break;
		}
		if (c < 32 && c != '\b' && c != '\r')
		{
			continue;
		}
		switch (c)
		{
			case '\r':
				std::cout << std::endl;
				break;
			case '\b':
				if (ret.size() > 0)
				{
					ret = ret.substr(0, ret.length() - 1);
					std::cout << "\b \b";
				}
				break;
			default:
				std::cout << c;
				ret += c;
				break;
		}
	}
	while (c != '\r');
	return ret;
}

std::vector<std::string> parseCommand(std::string cmd)
{
	std::vector<std::string> ret;
	std::string buf = "";
	bool quoteOn = false;
	bool isEscaping = false;
	for (int i = 0; i < cmd.length(); i += 1)
	{
		if (isEscaping)
		{
			buf += cmd[i];
			isEscaping = false;
		}
		else if (cmd[i] == '\\')
		{
			isEscaping = true;
			continue;
		}
		else if (cmd[i] == '"')
		{
			quoteOn = !quoteOn;
		}
		else if (cmd[i] == ' ' && !quoteOn && buf != "")
		{
			ret.insert(ret.end(), buf);
			buf = "";
		}
		else if (quoteOn || cmd[i] != ' ')
			buf += cmd[i];
	}
	if (buf.length() > 0)
	{
		ret.insert(ret.end(), buf);
		buf = "";
	}
	if (quoteOn)
	{
		// Continue line.
		std::vector<std::string> cont;
		cont.insert(cont.begin(), "__continue");
		return cont;
	}
	return ret;
}

bool packageIsOpen()
{
	return (openPackage != "" && currentFilesystem != NULL && fd != NULL);
}

void showCurrentOpenPackage()
{
	if (packageIsOpen())
		std::cout << "Currently opened package: " << openPackage << "." << std::endl;
	else
		std::cout << "No package currently open." << std::endl;
}

bool checkPackageOpen()
{
	if (!packageIsOpen())
	{
		std::cout << "There is no package open!" << std::endl;
		return false;
	}
	else
		return true;
}

bool checkArguments(std::string name, std::vector<std::string> args, int minargs, int maxargs)
{
	if (maxargs == -1) maxargs = minargs;
	// We subtract one because the first argument is the function call name.
	if (args.size() - 1 < minargs || args.size() - 1 > maxargs)
	{
		std::cout << "Invalid number of arguments to command '" << name << "'." << std::endl;
		std::cout << "Type 'help " << name << "' for usage details." << std::endl;
		return false;
	}
	return true;
}