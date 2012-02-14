do
    local gsub = string.gsub
    local HTML_ESC = {
        ["<"] = '&lt;', [">"] = '&gt;', ["&"] = '&amp;', ["\""] = '&quot;',
    }
    string.html = function (self)
        return (gsub(self, '[<>&"]', function (c) return HTML_ESC[c] end))
    end
end

local tmpl = loadfile("test/foo.qtmpl.lua")()
local out = io.open("test/out.got", "wb")

local info = {
        hello = "Hello &<world>\"'",
        list = { "foo", "bar", "baz" },
}
tmpl.generate(out, info)
