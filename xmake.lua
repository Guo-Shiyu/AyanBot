
-- enable Ayan Lua Interface
AYAN_LUA_INTERFACE = false

-- enable Ayan Python Interface
AYAN_PYTHON_INTERFACE = false

-- Python Interpreter Path
FOREIGN_INTERPRETER_PATH = [[D:\\...\\Anaconda]]

add_rules("mode.debug", "mode.debug")

if AYAN_LUA_INTERFACE then
    add_requires("sol2")
end

if AYAN_PYTHON_INTERFACE then
    add_requires("pybind11")
end

if not is_os("windows") then
    add_requires("fmt")
end
add_requires("libhv", {configs = {openssl = true}})

set_languages("c++latest")

target("AyanBot")
    set_kind("binary")
    add_files("src/*.cpp")
    add_files("src/core/*.cpp")
    add_packages("libhv")

    if not is_os("windows") then
        add_packages("fmt")
        set_languages("c++20")
        add_cxxflags("-fconcept")
    end

    if AYAN_LUA_INTERFACE then
        add_packages("sol2")
        add_defines("LUA_INTERFACE")
    end

    if (AYAN_PYTHON_INTERFACE) then 
        add_packages("pybind11")
        add_defines("PYTHON_INTERFACE")
        add_defines("INTERPRETER_PATH=\"".. FOREIGN_INTERPRETER_PATH .. "\"")
    end

    

--
-- If you want to known more usage about xmake, please see https://xmake.io
--
-- ## FAQ
--
-- You can enter the project directory firstly before building project.
--
--   $ cd projectdir
--
-- 1. How to build project?
--
--   $ xmake
--
-- 2. How to configure project?
--
--   $ xmake f -p [macosx|linux|iphoneos ..] -a [x86_64|i386|arm64 ..] -m [debug|release]
--
-- 3. Where is the build output directory?
--
--   The default output directory is `./build` and you can configure the output directory.
--
--   $ xmake f -o outputdir
--   $ xmake
--
-- 4. How to run and debug target after building project?
--
--   $ xmake run [targetname]
--   $ xmake run -d [targetname]
--
-- 5. How to install target to the system directory or other output directory?
--
--   $ xmake install
--   $ xmake install -o installdir
--
-- 6. Add some frequently-used compilation flags in xmake.lua
--
-- @code
--    -- add debug and release modes
--    add_rules("mode.debug", "mode.release")
--
--    -- add macro defination
--    add_defines("NDEBUG", "_GNU_SOURCE=1")
--
--    -- set warning all as error
--    set_warnings("all", "error")
--
--    -- set language: c99, c++11
--    set_languages("c99", "c++11")
--
--    -- set optimization: none, faster, fastest, smallest
--    set_optimize("fastest")
--
--    -- add include search directories
--    add_includedirs("/usr/include", "/usr/local/include")
--
--    -- add link libraries and search directories
--    add_links("tbox")
--    add_linkdirs("/usr/local/lib", "/usr/lib")
--
--    -- add system link libraries
--    add_syslinks("z", "pthread")
--
--    -- add compilation and link flags
--    add_cxflags("-stdnolib", "-fno-strict-aliasing")
--    add_ldflags("-L/usr/local/lib", "-lpthread", {force = true})
--
-- @endcode
--

