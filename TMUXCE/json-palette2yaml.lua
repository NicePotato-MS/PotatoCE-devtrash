local json = require("json")

local outYaml = ""

local outYaml = outYaml..[[palettes:
  - name: xterm256
    fixed-entries:]]

for k,v in ipairs(json.decode(io.open("xterm-colors.json","r"):read())) do
    outYaml = outYaml.."\n      - color: {index: "..tostring(k-1)
    outYaml = outYaml..", r: "..tostring(tonumber(v:sub(1,2),16))
    outYaml = outYaml..", g: "..tostring(tonumber(v:sub(3,4),16))
    outYaml = outYaml..", b: "..tostring(tonumber(v:sub(5,6),16))
    outYaml = outYaml.."}"

end

local outYaml = outYaml.."\n"

local outYaml = outYaml..[[palettes:
  - name: xubunterm
    fixed-entries:]]

for k,v in ipairs(json.decode(io.open("xubunterm.json","r"):read())) do
    outYaml = outYaml.."\n      - color: {index: "..tostring(k-1)
    outYaml = outYaml..", r: "..tostring(tonumber(v:sub(1,2),16))
    outYaml = outYaml..", g: "..tostring(tonumber(v:sub(3,4),16))
    outYaml = outYaml..", b: "..tostring(tonumber(v:sub(5,6),16))
    outYaml = outYaml.."}"

end

outYaml = outYaml..[[
    
outputs:
  - type: c
    include-file: gfx.h
    palettes:
      - xterm256
      - xubunterm]]

local convimg = io.open("src/gfx/convimg.yaml","w")
if convimg then
    convimg:write(outYaml)
end