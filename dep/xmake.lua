package("fmt")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "fmt"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()

package("gtest")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "gtest"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)
package_end()

package("hv")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "libhv/"))
    on_install(function (package)
        local configs = {}
        table.insert(configs, "-DCMAKE_BUILD_TYPE=" .. (package:debug() and "Debug" or "Release"))
        table.insert(configs, "-DBUILD_SHARED_LIBS=" .. (package:config("shared") and "ON" or "OFF"))
        import("package.tools.cmake").install(package, configs)
    end)

package_end()

package("pybind11")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "pybind11"))
package_end()

package("sol")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "sol"))
package_end()