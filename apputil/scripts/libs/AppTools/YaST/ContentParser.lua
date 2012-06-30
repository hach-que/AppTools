name "AppTools.YaST.ContentParser"
description [[Provides functionality for parsing YaST content files.]]
author "James Rhodes"
lastmodified "30th June, 2012"

-- Mark our variables as per instance.
context
{
	Parse = instance
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	Parse = public
	
	-- Private
}

function ContentParser:__init()
end

function ContentParser:Parse(data)
	function getkv(line)
		local result = { key = nil, value = {} };
		for v in line:gmatch("[^\ ]+") do
			if (result.key == nil) then
				result.key = v
			elseif (v == "") then
				-- pass
			else
				result.value[#result.value + 1] = v
			end
		end
		return result
	end

	local result = {}
	for v in data:gmatch("[^\r\n]+") do
		local kv = getkv(v)
		result[#result + 1] = kv
		result[kv.key] = kv.value
	end
	return result
end
