package("fmt")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "fmt"))
package_end()

package("gtest")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "gtest"))
package_end()

package("hv")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "hv"))
package_end()

package("pybind11")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "pybind11"))
package_end()

package("sol")
    add_deps("cmake")
    set_sourcedir(path.join(os.scriptdir(), "sol"))
package_end()

add_includedirs(
    ".",                   -- hv
    "fmt/include",
    "optionals/include",
    "result/include",
    "source_location/include",
    "sol/include",
    "pybind11/include", 
    { public = true }
)

-- if AyanConfig ~= nil then 
--     print(AyanConfig.Extensions.Python.InterpreterPath)
-- end 