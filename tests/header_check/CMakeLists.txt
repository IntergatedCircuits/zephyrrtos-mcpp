cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(zephyrrtos_mcpp_header_check)

option(USE_CLANG_TIDY "Run clang-tidy on the library headers during the build" OFF)

set(include_dir ${CMAKE_CURRENT_LIST_DIR}/../../include)
file(GLOB headers RELATIVE ${include_dir} ${include_dir}/zephyr/*.hpp)

# every header gets its own translation unit that includes nothing else,
# which proves that it is self-contained
foreach(header IN LISTS headers)
    string(MAKE_C_IDENTIFIER ${header} id)
    set(source ${CMAKE_CURRENT_BINARY_DIR}/header_check/${id}.cpp)
    file(CONFIGURE OUTPUT ${source}
        CONTENT "#include \"${header}\" // NOLINT(misc-include-cleaner)\n")
    target_sources(app PRIVATE ${source})
endforeach()
target_sources(app PRIVATE src/main.cpp)

target_compile_options(app PRIVATE -Wall -Wextra -Werror)

if(USE_CLANG_TIDY)
    # clang-tidy 22 or later is required for --removed-arg, which is needed with the gcc toolchains
    find_program(CLANG_TIDY_EXE NAMES clang-tidy-23 clang-tidy-22 clang-tidy REQUIRED)

    # only report the findings of the library headers (the checks come from its .clang-tidy)
    cmake_path(NORMAL_PATH include_dir OUTPUT_VARIABLE include_dir)
    string(REGEX REPLACE "([][.+*?^$|(){}\\])" "\\\\\\1" include_regex "${include_dir}")

    # clang doesn't find the target and the libstdc++ headers of gcc, so take them from the compiler
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} ${TOOLCHAIN_C_FLAGS} -xc++ -E -v -
        INPUT_FILE /dev/null OUTPUT_QUIET ERROR_VARIABLE compiler_output)
    string(REGEX MATCH "\nTarget: ([^\n]+)" _ "${compiler_output}")
    set(compiler_target ${CMAKE_MATCH_1})
    string(REGEX MATCH "#include <...> search starts here:\n(.*)\nEnd of search list." _
        "${compiler_output}")
    string(REPLACE "\n" ";" system_dirs "${CMAKE_MATCH_1}")

    set(clang_tidy_command
        ${CLANG_TIDY_EXE} --quiet
        --config-file=${include_dir}/../.clang-tidy
        --header-filter=^${include_regex}/
        --warnings-as-errors=*
        # gcc flags that clang rejects
        --removed-arg=-fno-freestanding
        --removed-arg=-fno-printf-return-value
        --removed-arg=-fno-reorder-functions
        --removed-arg=-mfp16-format=ieee
        # gcc flags that clang ignores
        --extra-arg=-Wno-unknown-warning-option
        --extra-arg=-Wno-unused-command-line-argument
        --extra-arg=-Wno-ignored-optimization-argument
        --extra-arg=--target=${compiler_target})
    foreach(dir IN LISTS system_dirs)
        string(STRIP "${dir}" dir)
        if(dir MATCHES "/c\\+\\+/")
            list(APPEND clang_tidy_command --extra-arg=-isystem${dir})
        endif()
    endforeach()
    set_target_properties(app PROPERTIES CXX_CLANG_TIDY "${clang_tidy_command}")
endif()
