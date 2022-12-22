#
# CMake file for jemalloc
#

function(do_build_jemalloc)
    set(jemalloc_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/deps/jemalloc)
    set(jemalloc_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/deps/jemalloc)
    set(jemalloc_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/deps/jemalloc/artifacts)

    list(APPEND jemalloc_CONFIG_ARGS --prefix=${jemalloc_INSTALL_DIR})
    list(APPEND jemalloc_CONFIG_ARGS --disable-initial-exec-tls)
    list(APPEND jemalloc_CONFIG_ARGS --without-export)
    #list(APPEND jemalloc_CONFIG_ARGS --disable-cxx)
    list(APPEND jemalloc_CONFIG_ARGS --with-jemalloc-prefix=je)
    #list(APPEND jemalloc_CONFIG_ARGS --enable-debug)

    if(NOT EXISTS ${jemalloc_SOURCE_DIR}/configure)
        execute_process(
                COMMAND ./autogen.sh
                WORKING_DIRECTORY ${jemalloc_SOURCE_DIR}
        )
    endif()

    include(ExternalProject)
    ExternalProject_Add(jemalloc-ext
            PREFIX ${jemalloc_BINARY_DIR}
            SOURCE_DIR ${jemalloc_SOURCE_DIR}
            CONFIGURE_COMMAND CC=${CMAKE_C_COMPILER} CXX=${CMAKE_CXX_COMPILER}
            <SOURCE_DIR>/configure ${jemalloc_CONFIG_ARGS}
            BINARY_DIR ${jemalloc_BINARY_DIR}
            BUILD_COMMAND $(MAKE)
            BUILD_ALWAYS TRUE
            BUILD_BYPRODUCTS ${jemalloc_LIBRARY}
            INSTALL_DIR ${jemalloc_INSTALL_DIR}
            INSTALL_COMMAND env -i $(MAKE) install
            TEST_COMMAND true
            )
endfunction()

macro(build_jemalloc)
    do_build_jemalloc()

    ExternalProject_Get_Property(jemalloc-ext INSTALL_DIR)

    set(JEMALLOC_INCLUDE_DIR ${INSTALL_DIR}/include)
    file(MAKE_DIRECTORY ${JEMALLOC_INCLUDE_DIR})
    set(JEMALLOC_STATIC_LIBRARY jemalloc-static)
    set(JEMALLOC_SHARED_LIBRARY jemalloc-shared)

    add_library(jemalloc-static STATIC IMPORTED)
    add_dependencies(jemalloc-static jemalloc-ext)

    set_target_properties(jemalloc-static PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${JEMALLOC_INCLUDE_DIR}
            IMPORTED_LINK_INTERFACE_LANGUAGES "C;CXX"
            IMPORTED_LOCATION ${INSTALL_DIR}/lib/libjemalloc_pic.a
            )

    add_library(jemalloc-shared SHARED IMPORTED)
    add_dependencies(jemalloc-shared jemalloc-ext)

    set_target_properties(jemalloc-shared PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES ${JEMALLOC_INCLUDE_DIR}
            IMPORTED_LOCATION ${INSTALL_DIR}/lib/libjemalloc.so
            )
endmacro()