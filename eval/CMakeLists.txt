file(GLOB_RECURSE eval_srcs *.cpp)
set(eval_bin ${PROJECT_NAME}-eval)
add_executable(${eval_bin} ${eval_srcs})

foreach(fpath ${eval_srcs})
    get_filename_component(tname ${fpath} NAME_WE)
    if(${tname} MATCHES "^eval_")
        string(SUBSTRING ${tname} 5 -1 tdisplay)
        add_eval(NAME ${tdisplay} COMMAND ${eval_bin} -\# [\#${tname}])
    endif()
endforeach()

target_link_libraries(${eval_bin} leftright)
set_target_properties(${eval_bin} PROPERTIES CXX_STANDARD 11)
target_include_directories(${eval_bin} SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/thirdparty/include")

add_compile_options(-Wall -Wpedantic -Wextra -Werror)
