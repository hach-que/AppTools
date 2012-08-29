name "AppTools.General.AppTypeCommandLine"
description [[A class that adds parser options to an ArgTable2 parser to deduce an application type.]]
author "James Rhodes"
lastmodified "16th February, 2012"
require "AppTools.General.AppType"
require "ArgTable2.Parser"

-- Mark our variables as per instance.
context
{
	GetTypeFromOptions = instance,
	m_ArgLibrary = instance,
	m_ArgApplication = instance,
	m_ArgEssential = instance,
	m_ArgOptional = instance,
	m_ArgSystem = instance,
	m_ArgUser = instance
};

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	__init = public,
	GetTypeFromOptions = public,

	-- Private
	m_ArgLibrary = private,
	m_ArgApplication = private,
	m_ArgEssential = private,
	m_ArgOptional = private,
	m_ArgSystem = private,
	m_ArgUser = private
};

-- Define our variables.
local m_ArgLibrary = nil
local m_ArgApplication = nil
local m_ArgEssential = nil
local m_ArgOptional = nil
local m_ArgSystem = nil
local m_ArgUser = nil

--- Creates a new AppTypeCommandLine, applying it to the specified parser.
--  @param parser The ArgTable2 parser.
function AppTypeCommandLine:__init(parser)
	if (not is(parser, "ArgTable2.Parser")) then
		error("AppTools.General.AppTypeCommandLine must be passed an ArgTable2 parser.");
		return nil
	end

	self.m_ArgLibrary = parser:AddLiteral0("l", "library", "The application is a library.");
	self.m_ArgApplication = parser:AddLiteral0("a", "application", "The application is not a library.");
	self.m_ArgEssential = parser:AddLiteral0("e", "essential", "The application is essential to system operation.");
	self.m_ArgOptional = parser:AddLiteral0("o", "optional", "The application is not essential.");
	self.m_ArgSystem = parser:AddLiteral0("s", "system", "The application is installed system-wide.");
	self.m_ArgUser = parser:AddLiteral0("u", "user", "The application resides in the user's directory.");
end

--- Determines the type of application from the options that were passed to the parser.
function AppTypeCommandLine:GetTypeFromOptions()
	if (not self.m_ArgLibrary and self.m_ArgEssential and self.m_ArgSystem) then
		return AppType.SYSTEM_ESSENTIAL_APP;
	elseif (self.m_ArgLibrary and self.m_ArgEssential and self.m_ArgSystem) then
		return AppType.SYSTEM_ESSENTIAL_LIB;
	elseif (not self.m_ArgLibrary and not self.m_ArgEssential and self.m_ArgSystem) then
		return AppType.SYSTEM_OPTIONAL_APP;
	elseif (self.m_ArgLibrary and not self.m_ArgEssential and self.m_ArgSystem) then
		return AppType.SYSTEM_OPTIONAL_LIB;
	elseif (not self.m_ArgLibrary and not self.m_ArgEssential and not self.m_ArgSystem) then
		return AppType.USER_OPTIONAL_APP;
	elseif (self.m_ArgLibrary and not self.m_ArgEssential and not self.m_ArgSystem) then
		return AppType.USER_OPTIONAL_LIB;
	else
		return nil
	end
end

