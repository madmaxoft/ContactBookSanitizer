#!/usr/bin/env lua

-- VcfAnonymizer.lua

-- Removes all personal info from a VCF file, leaving only the various field names




--- Extracts the name from the single line
-- Returns the substring of a_Line until the first non-dquoted colon
local function extractName(a_Line)
	local len = #a_Line
	local i = 1
	while (i < len) do
		local ch = a_Line:sub(i, i)
		if (ch == '"') then
			-- Skip until the next dquote:
			i = a_Line:find('"', i + 1)
		elseif (ch == ':') then
			return a_Line:sub(1, i)
		end
		i = i + 1
	end
	-- The line doesn't contain a non-dquoted colon. Most likely a wrongly folded value line (found on some phones' exporters):
	print("WARNING: Line doesn't contain a non-doublequoted colon. Probably a bug in the exporter.")
	print(a_Line)
	return ""
end





--- Anonymizes a single line
-- Returns the anonymized line - all data from the Value removed
local function anonymizeLine(a_Line)
	-- Skip empty lines altogether:
	if (a_Line == "") then
		return ""
	end
	-- Skip control lines:
	if (
		(a_Line == "BEGIN:VCARD") or
		(a_Line == "END:VCARD") or
		(a_Line:sub(1, 8) == "VERSION:")
	) then
		return a_Line .. "\r\n"
	end

	local res = extractName(a_Line)

	-- Remove known personalized substrings:
	res = res:gsub(';SORT%-AS=".-"',   ";SORT-AS-REMOVED")
	res = res:gsub(';SORT%-AS=[^:;]+', ";SORT-AS-REMOVED")

	return res .. "\r\n"
end





local args = { ... }
local inputFileName = args[1] or "Contacts.vcf"
local outputFileName = args[2] or "AnonymizedContacts.vcf"
print(string.format("Processing file %s into file %s", inputFileName, outputFileName))
local inputFile = assert(io.open(inputFileName, "rb"))
local outputFile = assert(io.open(outputFileName, "wb"))
local input = inputFile:read("*all")
inputFile:close()
input = input:gsub("\r\n\t", ""):gsub("\r\n ", "")  -- Unfold
input = input:gsub("(.-)\r\n", anonymizeLine)  -- Process each line
outputFile:write(input)
outputFile:close()
print("Done")
