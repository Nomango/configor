add_rules("mode.debug", "mode.release")
add_rules("plugin.compile_commands.autoupdate", {outputdir = "."})

target("configor")
    set_kind("headeronly")
    set_languages("c++17")

    add_headerfiles("include/configor/*.hpp")

target("test")
    set_kind("binary")
    set_languages("c++17")

    add_files("tests/*.cpp", "tests/test_basic_config/*.cpp", "tests/test_json/*.cpp")
    add_includedirs("tests/3rd-party", "include")

target("example")
    set_kind("binary")
    set_languages("c++17")

    add_files("examples/*.cpp")
    add_includedirs("include")