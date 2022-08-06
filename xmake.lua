AyanConfig = {

    Compilation = {
        -- "fastest" | "smallest"
        -- only take effect in release mode
        Optimize = "fastest",   
        
        -- "gcc" | "clang" | "msvc" | "default"
        ToolChain = "clang",
    },

    Extensions = {

        -- Python = {
        --     InterpreterPath = "<example path>",
        -- },

        -- Lua = {
            
        -- }
    }
}

add_rules("mode.debug", "mode.debug")

set_languages("cxx20")

-- switch toolchain 
if AyanConfig.Compilation.ToolChain ~= "default" then
    set_toolchains(AyanConfig.Compilation.ToolChain)
end 

-- parallel compile
set_policy("build.across_targets_in_parallel", true)

-- enable compile warning 
set_policy("build.warning", true)
set_warnings("all", "extra")


-- optimization setting and debug info 
if is_mode("debug") then
    set_symbols("debug")
elseif is_mode("release") then
    set_optimize("fastest")
    set_policy("build.optimization.lto", true)
    set_strip("all")
end 

add_includedirs("include", {public = true})

includes(
    "dep", 
    "test"
)

if AyanConfig.Extensions.Python then 
    print("using python extension")
end 

-- libayan / ayan.lib 
target("ayan")
    set_kind("static")
    add_files("src/*.cpp")
    remove_files("src/main.cpp")

-- HelloAyan.exe 
target("HelloAyan")
    set_kind("binary")
    add_deps("ayan")
    add_files("src/main.cpp")

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

