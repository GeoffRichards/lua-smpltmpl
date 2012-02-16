-- This will load the new copy of the library on Unix systems where it's built
-- with libtool.
package.cpath = ".libs/liblua-?.so;" .. package.cpath
local QTmpl = require 'qtemplate'

local TmplProc = QTmpl.new({ dirs = { 'test' } })
local tmpl, code = TmplProc:compile_template('foo')

-- Write out compiled template for debugging.
do
    local fh = assert(io.open("test/foo.qtmpl.lua", "wb"))
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
