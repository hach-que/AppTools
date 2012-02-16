name "AppTools.Logger"
description [[Provides logging facilities for AppTools scripts.]]
author "James Rhodes"
lastmodified "16th February, 2012"

-- Mark our variables as per instance.
context
{
	m_NameFull = instance,
	m_NameBlank = instance,
	m_Verbose = instance
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	__init = public,
	SetName = public,
	SetVerbose = public,
	ShowError = public,
	ShowWarning = public,
	ShowInfo = public,
	ShowSuccess = public,

	-- Private
	m_NameFull = private,
	m_NameBlank = private,
	m_Verbose = private,
	ShowMsg = private,
	CenterString = private
}

-- Define our variables.
local m_NameFull  = "apputil"
local m_NameBlank = "       "
local m_Verbose   = true

--- Creates a new logging instance.
function Logger:__init()
end

--- Sets the name of the application.
--  @param name The new name of the application.
function Logger:SetName(name)
	self.m_NameFull = name;
	self.m_NameBlank = "";
	for i = 0, name:len() do
		self.m_NameBlank = self.m_NameBlank .. " "
	end
end

--- Sets the verbosity of the logger.
--  @param verbose Whether informational messages will be shown.
function Logger:SetVerbose(verbose)
	self.m_Verbose = verbose;
end

--- Logs an error message.
--  @param msg The error message.
function Logger:ShowError(msg)
	self:ShowMsg("error", msg)
end

--- Logs a warning message.
--  @param msg The warning message.
function Logger:ShowWarning(msg)
	self:ShowMsg("warning", msg)
end

--- Logs an information message.
--  @param msg The informational message.
--  @param unignorable Whether the message should be shown regardless of the verbosity setting.
function Logger:ShowInfo(msg, unignorable)
	if (self.m_Verbose or unignorable == true) then
		self:ShowMsg("info", msg);
	end
end

--- Logs a success message.
--  @param msg The success message.
function Logger:ShowSuccess(msg)
	self:ShowMsg("success", msg);
end

--- Logs a message.
--  @param status The type of message.
--  @param msg The message.
function Logger:ShowMsg(status, msg)
	local msg = msg:gsub("\n", "\n" .. self.m_NameBlank .. "             ");
	print(self.m_NameFull .. ": [ " .. self:CenterString(status, 7) .. " ] " .. msg);
end

--- Centers a string.
--  @param str The string to center.
--  @param length The new length of the string.
function Logger:CenterString(str, length)
	local onLeft = false;
	while (str:len() < length) do
		if (onLeft) then
			str = " " .. str;
		else
			str = str .. " ";
		end
		onLeft = not onLeft;
	end
	return str;
end

