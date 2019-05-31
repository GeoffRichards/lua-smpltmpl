local M = { _NAME = 'smpltmpl' }
local ProcObj = { _NAME = 'smpltmpl processor object' }
ProcObj.__index = ProcObj
local TmplObj = { _NAME = 'smpltmpl template object' }
TmplObj.__index = TmplObj

local Priv = require 'smpltmpl_priv'

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
    return setmetatable({ dirs = opt.dirs, cache = {} }, ProcObj)
end

function ProcObj:compile_template (name)
    for _, dir in ipairs(self.dirs) do
        local filename = dir .. "/" .. name .. ".st"
        if Priv._file_exists(filename) then
            local mtime = Priv._file_mtime(filename)
            local cached = self.cache[filename]
            if cached and mtime == cached.mtime then
                return cached.tmpl, nil
            end

            local fh = assert(io.open(filename, "rb"))
            local data = assert(fh:read("a"))
            local code = Priv.compile(data, filename)
            local tmpl = assert(load(code, "compiled template code"))()
            local obj = setmetatable({ mod = tmpl, engine = self }, TmplObj)
            self.cache[filename] = { tmpl = obj, mtime = mtime }
            return obj, code
        end
    end
    error("template '" .. name .. "' not found in any include path")
end

function TmplObj:generate (out, info)
    self.mod:generate(self, M, out, info)
end

function TmplObj:_include (out, name, info)
    local tmpl = self.engine:compile_template(name)
    tmpl:generate(out, info)
end

return M
