name "AppTools.YaST.Repository"
description [[Provides functionality for reading and downloading from YaST repositories.]]
author "James Rhodes"
lastmodified "30th June, 2012"
require "socket"
require "socket.http"

-- Mark our variables as per instance.
context
{
	Probe = instance
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	Probe = public
	
	-- Private
}

function Repository:__init(url)
	self._url = url
	self._cache = { probed = false }
end

function Repository:Probe()
	if (self._cache.probed) then
		return
	end

	-- Download the content file.
	local data, code, headers = socket.http.request(self._url .. "/content")
	if (code ~= 200) then
		error("unable to fetch YaST content file")
	end

	-- Parse the content file.
	require "AppTools.YaST.ContentParser"
	local parser = AppTools.YaST.ContentParser()
	local values = parser:Parse(data)

	-- Create the settings cache.	
	self._cache.settings =
	{
		DistroName = values["DISTRIBUTION"][1],
		DistroVersion = values["VERSION"][1],
		ReleaseVersion = values["RELEASE"][1],
		Architectures = values["BASEARCHS"],
		Languages = values["LINGUAS"],
		DataURL = self._url .. "/" .. values["DATADIR"][1],
		DescriptionURL = self._url .. "/" .. values["DESCRDIR"][1]
	}

	-- Set probed status
	self._cache.probed = true
end

function Repository:PrintRepositoryInfo()
	if (not self._cache.probed) then
		self:Probe()
	end

	print("YaST Repository for " .. self._cache.settings.DistroName .. " " .. self._cache.settings.DistroVersion)
	io.write("Architectures: ")
	for i, v in ipairs(self._cache.settings.Architectures) do
		io.write(v .. " ")
	end
	print()
	io.write("Languages: ")
	for i, v in ipairs(self._cache.settings.Languages) do
		io.write(v .. " ")
	end
	print()
	print("Data URL: " .. self._cache.settings.DataURL)
	print("Description URL: " .. self._cache.settings.DescriptionURL)
end
