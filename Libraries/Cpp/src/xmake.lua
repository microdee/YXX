add_rules("mode.debug", "mode.release")

add_requires("pugixml")
add_requires("yaml-cpp")

target("yxx")
    set_kind("static")
    add_files("yxx.cpp")
    add_packages("yaml-cpp", {public = true})
    add_packages("pugixml", {public = true})
    add_headerfiles("yxx.h")