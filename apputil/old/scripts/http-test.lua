#!/bin/apputil

http = require("socket.http")
require("AppTools.Logger")
require("ArgTable2.Parser")

--local host = "www.w3.org"
--local file = "/TR/REC-html32.html"

--[[function download(host, file)
	local buffer = ""
	local c = assert(socket.connect(host, 80))
	local count = 0
	c:send("GET " .. file .. " HTTP/1.0\r\n\r\n")
	while true do
		local s, status = c:receive()
		if s ~= nil then
			count = count + string.len(s)
			buffer = buffer .. s
		end
		if status == "closed" then break end
	end
	c:close()
	print(file, count)
	print(buffer)
end

download("www.w3.org", "/TR/REC-html32.html")]]

data, code, headers = http.request("http://www.w3.org/TR/REC-html32.html")
print(data)

--[[log = AppTools.Logger()
log:SetName("test");
log:SetVerbose(true);
log:ShowError("This is an error.");
log:ShowSuccess("This is success!");
log:ShowWarning("This is a warning.");
log:ShowInfo("INFORMATION");
log:ShowWarning([This
is a multiline
warning.]);]]

--[[local parser = ArgTable2.Parser();
local arg_help    = parser:AddLiteral0("h", "help", "Shows this help.");
local arg_verbose = parser:AddLiteral0("v", nil, "Be verbose.");
local arg_file    = parser:AddFile1(nil, nil, "file", "The file to take as input.");
local arg_end     = parser:AddEnd();

local nerrors = parser:Parse();

-- Check to see if there were any errors.
if (nerrors > 0 and arg_help.count == 0) then
	parser:PrintSyntax("Usage: apputil type");
	parser:PrintErrors("apputil type");
	return 1
end

-- Check to see if the user requested showing the help
-- message.
if (arg_help.count == 1) then
	parser:PrintSyntax("Usage: apputil type");

	print("AppUtil Typer - Types the contents of files.\n");
	parser:PrintGlossary();
	return 0
end

-- Do our stuff.
if arg_verbose.count == 1 then
	print("Opening " .. arg_file.filename[0] .. " ... ")
end
local f = assert(io.open(arg_file.filename[0], "rb"));
local t = f:read("*all");
f:close();

if arg_verbose.count == 1 then
	print("File data is:")
end
print(t)]]

