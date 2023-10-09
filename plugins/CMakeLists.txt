macro(add_plugin_tde target)
    add_library(plugin-${target} SHARED)
    target_include_directories(plugin-${target} PUBLIC ../../lib/)

    cntp_init_plugin(libtdesktopenvironment plugin-${target} 20 plugin-${target})
    cntp_translate(plugin-${target})

    set_target_properties(plugin-${target} PROPERTIES
            OUTPUT_NAME ${target}Plugin
            FRAMEWORK FALSE)
endmacro()

add_subdirectory(x11)
add_subdirectory(wayland)
