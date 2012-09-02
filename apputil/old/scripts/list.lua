#!/bin/apputil

require "AppTools.Filesystem"
require "ArgTable2.Parser"

local parser = ArgTable2.Parser();
local arg_help    = parser:AddLiteral0("h", "help", "Shows this help.");
local arg_verbose = parser:AddLiteral0("v", nil, "Be verbose.");
local arg_file    = parser:AddFile1(nil, nil, "file", "The file to take as input.");
local arg_end     = parser:AddEnd();

local nerrors = parser:Parse();

-- Check to see if there were any errors.
if (nerrors > 0 and arg_help.count == 0) then
	parser:PrintSyntax("Usage: apputil list");
	parser:PrintErrors("apputil list");
	return 1
end

-- Check to see if the user requested showing the help
-- message.
if (arg_help.count == 1) then
	parser:PrintSyntax("Usage: apputil list");

	print("AppUtil List files in Package - Lists contents of packages.\n");
	parser:PrintGlossary();
	return 0
end

-- Do our stuff.
print("Opening " .. arg_file.filename[0] .. " ... ")

AppTools.Filesystem.Load(arg_file.filename[0]);
files = AppTools.Filesystem.Ls("/");
for i, v in ipairs(files) do
	print(files[i])
end
AppTools.Filesystem.Unload();
