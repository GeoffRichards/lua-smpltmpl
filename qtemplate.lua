local M = { _NAME = 'qtemplate' }
local Priv = require 'qtemplate_priv'

do
    local gsub = string.gsub
    local HTML_ESC = {
        ["<"] = '&lt;', [">"] = '&gt;', ["&"] = '&amp;', ["\""] = '&quot;',
    }
    string.html = function (self)
        return (gsub(self, '[<>&"]', function (c) return HTML_ESC[c] end))
    end
end

function M.compile (filename)
    Priv.compile(filename, "tmp.lua")
    local tmpfh = assert(io.open("tmp.lua", "rb"))
    local code = assert(tmpfh:read("*a"))
    tmpfh:close()
    local tmpl = assert(loadstring(code, "compiled template code"))()
    return tmpl, code
end

return M
