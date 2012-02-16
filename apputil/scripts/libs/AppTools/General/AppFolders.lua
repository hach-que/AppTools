name "AppTools.General.AppFolders"
description [[A static class that resolves an application type to the folder where it belongs.]]
author "James Rhodes"
lastmodified "16th February, 2012"
require "AppTools.General.AppType"

-- Mark our variables as per instance.
context
{
	GetFolder = static
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	GetFolder = public

	-- Private
}

--- Resolves an application type to the specified folder it should be stored in.
--  @param type The application type.
function AppFolders:GetFolder(type)
	if (type == AppTools.General.AppType.SYSTEM_ESSENTIAL_APP) then
		return "/pkg/system/applications";
	elseif (type == AppTools.General.AppType.SYSTEM_ESSENTIAL_LIB) then
		return "/pkg/system/libraries";
	elseif (type == AppTools.General.AppType.SYSTEM_OPTIONAL_APP) then
		return "/pkg/user/applications";
	elseif (type == AppTools.General.AppType.SYSTEM_OPTIONAL_LIB) then
		return "/pkg/user/libraries";
	elseif (type == AppTools.General.AppType.SYSTEM_GLOBAL_APP) then
		return "/";
	elseif (type == AppTools.General.AppType.SYSTEM_HW_DRIVER) then
		return "/pkg/system/hwdrivers";
	elseif (type == AppTools.General.AppType.USER_OPTIONAL_APP) then
		return os.getenv("HOME") .. "/Applications";
	elseif (type == AppTools.General.AppType.USER_OPTIONAL_LIB) then
		return os.getenv("HOME") .. "/Applications/.Libraries";
	elseif (type == AppTools.General.AppType.APPLICATION_EXTENSION) then
		return nil;
	else
		return nil;
	end
end



