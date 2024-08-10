---@diagnostic disable: need-check-nil

-- Created by NicePotato to convert limited amount of monospaced fonts to c
-- Not made for global usage
-- Only compression is packing bits in bytes
-- Not very well tested or made

local convfont_regu = io.open("src/fonts/drmono-10-normal.convfont","r"):read("*a")
local convfont_bold = io.open("src/fonts/drmono-10-bold.convfont","r"):read("*a")

string["split"] = function(str,delimeter)
    local lines = {}
    for line in str:gmatch("[^"..delimeter.."]+") do
        table.insert(lines, line)
    end
    return lines
end

string["trim"] = function(str)
    local trimmedString = str:gsub("^%s+", "")
    trimmedString = trimmedString:gsub("%s+$", "")

    return trimmedString
end

local function printTable(table,depth, key)
    depth = depth or 1

    local function textBrackets()
        if #table == 0 then
            return "{}"
        else
            return "{"
        end
    end
    if key then
        print(string.rep("\t",depth-1).."["..tostring(key).."] = "..textBrackets())
    else
        print(string.rep("\t",depth-1)..textBrackets())
    end
    for k,v in pairs(table) do
        if type(v) == "table" then
            printTable(v,depth+1,k)
        else
            print(string.rep("\t",depth).."["..tostring(k).."] = "..tostring(v))
        end
    end
    if #table > 0 then
        print(string.rep("\t",depth-1).."}")
    end
end

local function newFont()
    local font = {
        double_width = false,
        width = 0,
        height = 0,
        space_above = 0,
        characters = {},
        packed_bytes = {}
    }

    return font
end

local font
local lineArgs
local processMode
local currentChar
local charY
local process1 = {
    ["height"] = function()
        font.height = font.height+tonumber(lineArgs[2]:trim())
    end,
    ["fixed width"] = function()
        font.width = font.width+tonumber(lineArgs[2]:trim())
    end,
    ["double width"] = function()
        if lineArgs[2]:trim():lower() == "true" then
            font.double_width = 2
        else
            font.double_width = 1
        end
    end,
    ["space above"] = function()
        local space = tonumber(lineArgs[2]:trim())
        font.space_above = space
    end,
    ["code point"] = function ()
        if currentChar == 57 then
            currentChar = 58
        else
            if lineArgs[3] then
                lineArgs[2] = lineArgs[2]..lineArgs[3]
            end
            local point = lineArgs[2]:trim()
            if point:sub(1,1) == "'" then
                if point:sub(2,2) == "\\" then
                    currentChar = string.byte(point:sub(3,3))
                else
                    currentChar = string.byte(point:sub(2,2))
                end
            else
                currentChar = tonumber(point)
            end
        end
        charY = 1
        font.characters[currentChar] = {}
    end,
    ["data"] = function ()
        processMode = 2
    end
}

local function processFont(font_str)
    font = newFont()
    if type(font_str) ~= "string" then return end
    
    local lines = font_str:split("\r\n")
    processMode = 1
    for _,line in ipairs(lines) do
        if processMode == 1 then
            lineArgs = line:split(":")
            if process1[lineArgs[1]:lower()] then
                process1[lineArgs[1]:lower()]()
            end
        elseif processMode == 2 then
            font.characters[currentChar][charY] = {}
            for x = 1,font.width do
                if line:sub(x*font.double_width,x*font.double_width) == " " then
                    font.characters[currentChar][charY][x] = 0
                else
                    font.characters[currentChar][charY][x] = 1
                end
            end
            charY = charY+1
            if charY > font.height then
                processMode = 1
            end
        end
    end

    local workingByte = ""
    for char = 0,255 do
        for y = font.space_above+1,font.height do
            for x=1,font.width-1 do
                workingByte = workingByte..tostring(font.characters[char][y][x])
                if workingByte:len() == 8 then
                    font.packed_bytes[#font.packed_bytes+1] = tonumber(workingByte,2)
                    workingByte = ""
                end
            end
        end
    end
    if workingByte:len() > 0 then
        workingByte = workingByte..string.rep("0",8-workingByte:len())
        font.packed_bytes[#font.packed_bytes+1] = tonumber(workingByte,2)
        workingByte = ""
    end

    return font
end

local function exportFont(font,name,out)
    local fontOut = "unsigned char "..name.."[] = {"
    for k,byte in ipairs(font.packed_bytes) do
        if k == #font.packed_bytes then
            fontOut = fontOut..tostring(byte.."};")
        else
            fontOut = fontOut..tostring(byte..",")
        end
    end
    io.open(out..".c","w+"):write(fontOut)
    local headerName = "HEADER_PFONT_"..name:upper()
    local fontHeader = "#ifndef "..headerName.."\n#define "..headerName.."\n\n"
    fontHeader = fontHeader.."#define "..name:upper().."_X_SIZE "..tostring(font.width-1).."\n"
    fontHeader = fontHeader.."#define "..name:upper().."_Y_SIZE "..tostring(font.height-font.space_above).."\n"
    fontHeader = fontHeader.."\n"
    fontHeader = fontHeader.."extern unsigned char "..name.."[];"
    fontHeader = fontHeader.."\n\n#endif"
    io.open(out..".h","w+"):write(fontHeader)
end

exportFont(processFont(convfont_regu),"drmono_10_regular","src/pfonts/drmono_10_regular")
exportFont(processFont(convfont_bold),"drmono_10_bold","src/pfonts/drmono_10_bold")

