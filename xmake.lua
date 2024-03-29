AyanConfig = {

    Compilation = { 
        -- "gcc" | "clang" | "msvc" | "default"
        ToolChain = "default",

        -- */AyanBot/build 
        OutputPath = "build",
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

set_languages("c++20")

-- switch toolchain 
if AyanConfig.Compilation.ToolChain ~= "default" then
    set_toolchains(AyanConfig.Compilation.ToolChain)
end 

includes("dep", "test")

add_requires("hv", "fmt")

-- libayan | ayan.lib 
target("ayan")
    set_kind("static")
    add_files("src/*.cpp", "src/*/*.cpp")
    
    -- include/ayan
    add_includedirs("include", { public = true })

    -- dep/*
    add_includedirs(
        -- libhv
        "dep", "dep/libhv", "dep/libhv/evpp", "dep/libhv/base", "dep/libhv/ssl", "dep/libhv/event", "dep/libhv/cpputil", "dep/libhv/http",

        -- others 
        "dep/fmt/include",
        "dep/optional/include",
        "dep/result/include",
        "dep/source_location/include",
        "dep/sol/include",
        "dep/pybind11/include", 
        { public = true }
    )
    remove_files("src/main.cpp")

    set_targetdir(AyanConfig.Compilation.OutputPath.."/lib")
    if is_plat("linux") then 
        add_files("build/.packages/**/*.a")
    elseif is_plat("windows") then 
        add_files("build/.packages/**/*.lib")
    end 

-- HelloAyan.exe 
target("HelloAyan")
    set_kind("binary")
    add_deps("ayan")
    add_files("src/main.cpp")
    set_strip("all")
    set_policy("build.optimization.lto", true)
    set_targetdir(AyanConfig.Compilation.OutputPath.."/app")

    if is_plat("windows") then 
        after_link(function (target) 
            os.cp("build/.packages/**/*.dll", "build/app")            
        end)
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

