-- vim: set filetype=lua:

-- Overwrite our loading functionality.
local old = package.loaders;
package.loaders = {}

package.loaders[1] = function(module, env)
	module = _APPUTIL_LIB_PATH .. module

	-- Declare our variables local so they don't
	-- interfere with the main program.
	if (env == nil) then
		env = _G
	end
	
	local path, ext, found_path, e, fhandle
	local chunk, lmodule, _T, lfmodulepath
	local indent, l, n, ldotpos, classname
	local namespace, class_funcs, class, mt
	local __init, f, v, k, tdotpos, __base, i
	local class_inheritance = {}
	local inherit_classname, inherit_namespaces
	local inheritance_sandbox, i2
	local function func_inherit(class)
		if (lmodule._T == nil) then
			lmodule._T = {}
		end
		
		table.insert(class_inheritance, class)
	end
	local function func_name(name)
		local ldotpos, tdotpos, classname, namespaces
		local z, x, c
		ldotpos = string.len(name)
		tdotpos = string.len(name)
		classname = nil
		namespaces = {}
		while (ldotpos > 0) do
			if (string.sub(name,ldotpos,ldotpos) == ".") then
				if (classname == nil) then
					classname = string.sub(name, ldotpos + 1, tdotpos)
				else
					namespaces[#namespaces + 1] = string.sub(name, ldotpos + 1, tdotpos);
				end
				tdotpos = ldotpos - 1
			end
			ldotpos = ldotpos - 1
		end
		if (tdotpos > 0) then
			namespaces[#namespaces + 1] = string.sub(name, ldotpos + 1, tdotpos);
		end
		z = {}
		for c, x in ipairs(namespaces) do
			table.insert(z,1,x)
		end
		namespaces = z
		
		if (lmodule._T == nil) then
			lmodule._T = {}
		end
		
		lmodule._T["NAME"] = classname
		if (#namespaces > 0) then
			lmodule._T["NAMESPACE"] = namespaces
		else
			lmodule._T["NAMESPACE"] = nil
		end
	end
	local function func_description(desc)
		if (lmodule._T == nil) then
			lmodule._T = {}
		end
		
		lmodule._T["DESCRIPTION"] = desc
	end
	local function func_author(author)
		if (lmodule._T == nil) then
			lmodule._T = {}
		end
		
		lmodule._T["AUTHOR"] = author
	end
	local function func_lastmodified(date)
		if (lmodule._T == nil) then
			lmodule._T = {}
		end
		
		lmodule._T["LASTMODIFIED"] = date
	end
	local function func_context(data)
		if (lmodule._T == nil) then
			lmodule._T = {}
		end
		
		lmodule._T["CONTEXT"] = data
	end
	local function func_visibility(data)
		if (lmodule._T == nil) then
			lmodule._T = {}
		end

		lmodule._T["VISIBILITY"] = data
	end

	-- Define some global variables
	local var_instance = 0
	local var_static = 1
	local var_public = 0
	local var_private = 1
	
	-- Replace the "." in the requested module with
	-- backslashes.
	path = string.gsub(module, "%.", "/")
	path = "./" .. path
	ext = {"lua"}
	found_path = nil
	
	for k, e in pairs(ext) do
		fhandle = io.open(path .. "." .. e, "r")
		if (fhandle ~= nil) then
			fhandle:close()
			found_path = path .. "." .. e
			break
		end
	end
	
	if (found_path == nil) then
		return nil, "Unable to locate module at " .. path .. ".lua."
	end
	
	-- Since the file exists, we're now going to load it.
	chunk, cerror = loadfile(found_path)
	if (chunk == nil) then
		print("ERR : Module " .. module .. " contains syntax errors and cannot be included in the program.")
		print("ERR : " .. cerror)
		return nil, "Module " .. module .. " contains syntax errors and cannot be included in the program."
	end
	
	-- Isolate the class name from the namespace component
	ldotpos = string.len(module)
	while (ldotpos > 0) do
		if (string.sub(module,ldotpos,ldotpos) == ".") then
			break
		end
		ldotpos = ldotpos - 1
	end
	classname = string.sub(module, ldotpos + 1)
	if (ldotpos ~= 0) then
		namespace = string.sub(module, 1, ldotpos - 1)
	else
		namespace = ""
	end
	
	-- Run the chunk() function inside a sandbox, so we can inspect the module.
	lmodule = {}
	lmodule[classname] = {}
	lmodule["inherits"] = func_inherit
	lmodule["name"] = func_name
	lmodule["description"] = func_description
	lmodule["author"] = func_author
	lmodule["lastmodified"] = func_lastmodified
	lmodule["context"] = func_context
	lmodule["visibility"] = func_visibility
	lmodule["require"] = require
	setfenv(chunk, lmodule)
	chunk()

	-- Check to see if _T exists, if it doesn't, show that the module can't be loaded.
	if (lmodule["_T"] == nil) then
		print("ERR : Module " .. module .. " does not specify module information.")
		return nil, "Module " .. module .. " does not specify module information."
	end
	
	-- Move the _T table from the code block, into a local variable.
	_T = lmodule["_T"]
	lmodule["_T"] = nil
	
	-- Sanitize the _T variable we will use.
	_T["NAME"]			= tostring(_T["NAME"])
	if (_T["DESCRIPTION"] ~= nil) then
		_T["DESCRIPTION"]	= tostring(_T["DESCRIPTION"])
	end
	if (_T["AUTHOR"] ~= nil) then
		_T["AUTHOR"]		= tostring(_T["AUTHOR"])
	end
	if (_T["LASTMODIFIED"] ~= nil) then
		_T["LASTMODIFIED"]	= tostring(_T["LASTMODIFIED"])
	end
	
	-- Verify that the module is located at the correct location (NAMESPACE "." NAME == module)
	if (_T["NAMESPACE"] ~= nil) then
		lfmodulepath = table.concat(_T["NAMESPACE"], ".") .. "." .. _T["NAME"]
		if (_APPUTIL_LIB_PATH .. lfmodulepath ~= module) then
			print("ERR : Module name mismatch.  Loaded from " .. module .. ", but code specifies " .. lfmodulepath .. ".")
			return nil, "Module name mismatch.  Loaded from " .. module .. ", but code specifies " .. lfmodulepath .. "."
		end
	else
		lfmodulepath = _T["NAME"]
	end
	
	-- Now create the namespace tables if required.
	l = env
	if (_T["NAMESPACE"] ~= nil) then
		for k, n in pairs(_T["NAMESPACE"]) do
			l[n] = {}
			l = l[n]
		end
	end
	
	-- Get the class functions.
	class_funcs = lmodule[classname]
	lmodule[classname] = nil
	__init = nil
	__base = nil
	
	-- Build up the class.
	class = {}
	if (#class_inheritance > 0) then
		-- We need to evaluate all of the inherited class
		-- in order, to build up.
		for k, v in pairs(class_inheritance) do
			-- Isolate the class name from the namespace component
			print("INFO: Loading " .. v .. " in required module.  Namespaces / classes should not leak.")
			ldotpos = string.len(v)
			tdotpos = string.len(v)
			inherit_classname = nil
			inherit_namespaces = {}
			while (ldotpos > 0) do
				if (string.sub(v,ldotpos,ldotpos) == ".") then
					if (inherit_classname == nil) then
						inherit_classname = string.sub(v, ldotpos + 1, tdotpos)
					else
						table.insert(inherit_namespaces,1,string.sub(v, ldotpos + 1, tdotpos))
					end
					tdotpos = ldotpos - 1
				end
				ldotpos = ldotpos - 1
			end
			if (tdotpos > 0) then
				table.insert(inherit_namespaces,1,string.sub(v, ldotpos + 1, tdotpos))
			end
			
			inheritance_sandbox = {}
			package.loaders[1](v, inheritance_sandbox)
			
			-- Load the class into i.
			i = inheritance_sandbox
			for i2, n in pairs(inherit_namespaces) do
				i = i[n]
			end
			i = i[inherit_classname]
			
			-- Now copy all of the inherited classes functions.
			for n, f in pairs(i) do
				if (n == "__init") then
					__base = f
					setfenv(__base, env)
				elseif (type(f) == "function") then
					class[n] = f
					setfenv(class[n], env)
				elseif (type(f) ~= "function") then
					class[n] = f
				end
			end
		end
	end
	for n, f in pairs(class_funcs) do
		if (n == "__init") then
			__init = f
			setfenv(__init, env)
		else
			class[n] = f
			setfenv(class[n], env)
		end
	end
	for n, v in pairs(lmodule) do
		if (n ~= "inherits"
			and n ~= "name"
			and n ~= "description"
			and n ~= "author"
			and n ~= "lastmodified"
			and n ~= "context"
			and n ~= "visibility"
			and n ~= "require") then
			if (type(v) == "function") then
				print("WARN: " .. lfmodulepath .. ":0: Function " .. n .. " defined without class context.  It is not included in the class definition.")
			else
				-- Make the specified variable a static variable.
				class[n] = v
			end
		end
	end
	mt = {}
	mt.__call = function(class_tbl, ...)
		local obj = {}
		setmetatable(obj, class)
		if (__init ~= nil) then
			__init(obj, ...)
		elseif (__base ~= nil) then
			__base(obj, ...)
		end
		return obj
	end
	class.__index = class
	class.__init = __init
	class.__base = __base
	setmetatable(class, mt)
	
	-- Now put the newly generated class in the namespace.
	l[classname] = class
	
	return function()
	end
end

-- Add native module support again.
package.cpath = _APPUTIL_LIB_PATH .. "/.native/?.so"
package.loaders[2] = old[3];
