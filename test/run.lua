-- This will load the new copy of the library on Unix systems where it's built
-- with libtool.
package.cpath = ".libs/liblua-?.so;" .. package.cpath
local Tmpl = require 'smpltmpl'

local proc = Tmpl.new({ dirs = { 'test' } })
local tmpl, code = proc:compile_template('foo')
local tmpl2, code2 = proc:compile_template('foo')
assert(tmpl2 == tmpl)
assert(code2 == nil)

-- Write out compiled template for debugging.
do
    local fh = assert(io.open("test/foo.st.lua", "wb"))
    fh:write(code)
    fh:close()
end

local info = {
    hello = "Hello &<world>\"'",
    list = { "foo", "bar", "baz" },
}

local out = assert(io.open("test/out.got", "wb"))
tmpl:generate(out, info)
out:close()
