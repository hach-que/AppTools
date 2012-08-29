name "ArgTable2.Parser"
description [[Exposes the argtable2 library to Lua.]]
author "James Rhodes"
lastmodified "15th February, 2012"
require "argtable2"

-- Mark our variables as per instance.
context
{
	m_Options = instance,
}

-- Mark our visiblity for variables and functions.
visibility
{
	-- Public
	__init = public,

	-- Private
	m_Options = private,
	m_ArgEnd = private,
	AddOption = private,
	WrapOption = private,
	GenerateWrapper = private,
	GetRawOptions = private
}

-- Define our variables.
local m_Options   = {}
local m_ArgEnd    = nil;

--- Creates a new parser instance.
function Parser:__init()
	self.m_Options = {};
	self.m_ArgEnd = nil;
end

--[[
-- Add option functions
--]]
function Parser:AddLiteral0(short, long, description)
	local opts = argtable2.arg_litw0(short, long, nil, description);
	self:AddOption("literal", opts);
	return self:WrapOption("literal", opts);
end

function Parser:AddLiteral1(short, long, description)
	local opts = argtable2.arg_litw1(short, long, nil, description);
	self:AddOption("literal", opts);
	return self:WrapOption("literal", opts);
end

function Parser:AddLiteralN(short, long, mincount, maxcount, description)
	local opts = argtable2.arg_litwn(short, long, nil, mincount, maxcount, description);
	self:AddOption("literal", opts);
	return self:WrapOption("literal", opts);
end

function Parser:AddNumber0(short, long, datatype, description)
	local opts = argtable2.arg_dbl0(short, long, datatype, description);
	self:AddOption("number", opts);
	return self:WrapOption("number", opts);
end

function Parser:AddNumber1(short, long, datatype, description)
	local opts = argtable2.arg_dbl1(short, long, datatype, description);
	self:AddOption("number", opts);
	return self:WrapOption("number", opts);
end

function Parser:AddNumberN(short, long, datatype, mincount, maxcount, description)
	local opts = argtable2.arg_dbln(short, long, datatype, mincount, maxcount, description);
	self:AddOption("number", opts);
	return self:WrapOption("number", opts);
end

function Parser:AddString0(short, long, datatype, description)
	local opts = argtable2.arg_str0(short, long, datatype, description);
	self:AddOption("string", opts);
	return self:WrapOption("string", opts);
end

function Parser:AddString1(short, long, datatype, description)
	local opts = argtable2.arg_str1(short, long, datatype, description);
	self:AddOption("string", opts);
	return self:WrapOption("string", opts);
end

function Parser:AddStringN(short, long, datatype, mincount, maxcount, description)
	local opts = argtable2.arg_strn(short, long, datatype, mincount, maxcount, description);
	self:AddOption("string", opts);
	return self:WrapOption("string", opts);
end

function Parser:AddFile0(short, long, datatype, description)
	local opts = argtable2.arg_file0(short, long, datatype, description);
	self:AddOption("file", opts);
	return self:WrapOption("file", opts);
end

function Parser:AddFile1(short, long, datatype, description)
	local opts = argtable2.arg_file1(short, long, datatype, description);
	self:AddOption("file", opts);
	return self:WrapOption("file", opts);
end

function Parser:AddFileN(short, long, datatype, mincount, maxcount, description)
	local opts = argtable2.arg_filen(short, long, datatype, mincount, maxcount, description);
	self:AddOption("file", opts);
	return self:WrapOption("file", opts);
end

function Parser:AddEnd(errors)
	if (self.m_ArgEnd == nil) then
		local opts = argtable2.arg_end(errors);
		self:AddOption("end", opts);
		self.m_ArgEnd = opts;
		return self:WrapOption("end", opts);
	else
		error("You can't add two end elements to an ArgTable2 parser.");
	end
end

--[[
-- General functions
--]]

function Parser:Parse()
	local opts = self:GetRawOptions();
	local nerrors = argtable2.arg_parse(opts);
	return nerrors;
end

function Parser:PrintSyntax(lead, sep)
	argtable2.arg_print_syntax(lead, self:GetRawOptions(), sep);
end

function Parser:PrintErrors(name)
	argtable2.arg_print_errors(self.m_ArgEnd, name);
end

function Parser:PrintGlossary(fmt)
	argtable2.arg_print_glossary(self:GetRawOptions(), fmt);
end

--[[
-- Internal functions
--]]

function Parser:AddOption(typ, opt)
	self.m_Options[#self.m_Options + 1] = {typ, opt};
end

function Parser:WrapOption(typ, opt)
	local g_count = function() return argtable2.arg_g_count(opt); end
	local g_dval = function(idx) return argtable2.arg_g_dval(opt, idx); end
	local g_sval = function(idx) return argtable2.arg_g_sval(opt, idx); end
	local g_filename = function(idx) return argtable2.arg_g_filename(opt, idx); end
	local g_basename = function(idx) return argtable2.arg_g_basename(opt, idx); end
	local g_extension = function(idx) return argtable2.arg_g_extension(opt, idx); end

	if (typ == "literal") then
		return self:GenerateWrapper({ count = g_count });
	elseif (typ == "number") then
		return self:GenerateWrapper({ value = g_dval });
	elseif (typ == "string" or typ == "regex") then
		return self:GenerateWrapper({ value = g_sval });
	elseif (typ == "file") then
		return self:GenerateWrapper({ filename = g_filename, basename = g_basename, extension = g_extension });
	else
		return nil;
	end
end

function Parser:GenerateWrapper(methods)
	local t = {};
	local mt = {};
	mt.__index = function(t, key)
		for mKey, mValue in pairs(methods) do
			if (key == mKey) then
				if (key == "count") then
					return mValue();
				else
					-- Need to generate another indexable table.
					local tG = {};
					local mtG = {};
					mtG.__index = function(tt, idx)
						return mValue(idx);
					end
					return setmetatable(tG, mtG);
				end
			end
		end
		return nil;
	end
	return setmetatable(t, mt);
end

function Parser:GetRawOptions()
	local opts = {};
	for i, value in ipairs(self.m_Options) do
		opts[#opts + 1] = value[2];
	end
	return opts
end

