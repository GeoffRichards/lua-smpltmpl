local M = { _NAME = 'qtemplate' }
local ProcObj = { _NAME = 'qtemplate processor object' }
ProcObj.__index = ProcObj
local TmplObj = { _NAME = 'qtemplate template object' }
TmplObj.__index = TmplObj

local Priv = require 'qtemplate_priv'

do
    local gsub = string.gsub
    local HTML_ESC = {
        ["<"] = '&lt;', [">"] = '&gt;', ["&"] = '&amp;', ["\""] = '&quot;',
    }
    function M.escape_html (s)
        return (gsub(s, '[<>&"]', function (c) return HTML_ESC[c] end))
    end
end

function M.new (opt)
    if type(opt) ~= 'table' or type(opt.dirs) ~= 'table' or #opt.dirs < 1 then
        error("'dirs' option is required, and it must be a table")
    end
    return setmetatable({ dirs = opt.dirs }, ProcObj)
end

function ProcObj:compile_template (name)
    for _, dir in ipairs(self.dirs) do
        local filename = dir .. "/" .. name .. ".qtmpl"
        if Priv.file_exists(filename) then
            local fh = assert(io.open(filename, "rb"))
            local data = assert(fh:read("*a"))
            local code = Priv.compile(data, filename)
            local tmpl = assert(loadstring(code, "compiled template code"))()
            local obj = { mod = tmpl, engine = self }
            return setmetatable(obj, TmplObj), code
        end
    end
    error("template '" .. name .. "' not found in any include path")
end

function TmplObj:generate (out, info)
    self.mod:generate(M, out, info)
end

return M
