name "AppTools.General.AppType"
description [[An enumeration describing the type of an application.]]
author "James Rhodes"
lastmodified "16th February, 2012"

-- Mark our variables as per instance.
context
{
	SYSTEM_ESSENTIAL_APP = static,
	SYSTEM_ESSENTIAL_LIB = static,
	SYSTEM_OPTIONAL_APP = static,
	SYSTEM_OPTIONAL_LIB = static,
	SYSTEM_HW_DRIVER = static,
	SYSTEM_GLOBAL_APP = static,
	USER_OPTIONAL_APP = static,
	USER_OPTIONAL_LIB = static,
	APPLICATION_EXTENSION = static
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	SYSTEM_ESSENTIAL_APP = public,
	SYSTEM_ESSENTIAL_LIB = public,
	SYSTEM_OPTIONAL_APP = public,
	SYSTEM_HW_DRIVER = public,
	SYSTEM_GLOBAL_APP = public,
	USER_OPTIONAL_APP = public,
	APPLICATION_EXTENSION = public

	-- Private
}

-- Essential system application (i.e. binutils).
local SYSTEM_ESSENTIAL_APP = 0;
local SYSTEM_ESSENTIAL_LIB = 1;
local SYSTEM_OPTIONAL_APP = 2;
local SYSTEM_HW_DRIVER = 3;
local SYSTEM_GLOBAL_APP = 4;
local USER_OPTIONAL_APP = 5;
local APPLICATION_EXTENSION = 6;


