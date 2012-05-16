name "AppTools.Environment"
description [[Provides environment analysis tools for AppTools scripts.]]
author "James Rhodes"
lastmodified "16th February, 2012"

-- Mark our variables as per instance.
context
{
	CheckBinaries = static
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	CheckBinaries = public

	-- Private
}

--- Checks to ensure that the list of binaries are in PATH.
--  @param apps An indexed table of binary files to look for.
function Environment.CheckBinaries(apps)
	local paths = os.getenv("PATH");
	local found = {}
	for i, app in ipairs(apps) do
		found[app] = false
	end
	for i, path in ipairs(paths) do
		for ii, app in ipairs(apps) do
			local f = io.open(path + "/" + app, "r");
			if (f ~= nil) then
				f:close();
				found[app] = true
			end
		end
	end
	return found
end


