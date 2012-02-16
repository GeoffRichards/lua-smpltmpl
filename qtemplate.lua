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
    local code = Priv.compile(filename)
    local tmpl = assert(loadstring(code, "compiled template code"))()
    return tmpl, code
end

return M
