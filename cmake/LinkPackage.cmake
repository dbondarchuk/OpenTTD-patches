function(link_package NAME)
    cmake_parse_arguments(LP "ENCOURAGED" "TARGET" "" ${ARGN})

    if (${NAME}_FOUND)
        string(TOUPPER "${NAME}" UCNAME)
        add_definitions(-DWITH_${UCNAME})
        if (LP_TARGET AND TARGET ${LP_TARGET})
            target_link_libraries(openttd ${LP_TARGET})
            message(STATUS "${NAME} found -- -DWITH_${UCNAME} -- ${LP_TARGET}")
        else()
            include_directories(${${NAME}_INCLUDE_DIRS} ${${NAME}_INCLUDE_DIR})
            target_link_libraries(openttd ${${NAME}_LIBRARIES} ${${NAME}_LIBRARY})
            message(STATUS "${NAME} found -- -DWITH_${UCNAME} -- ${${NAME}_INCLUDE_DIRS} ${${NAME}_INCLUDE_DIR} -- ${${NAME}_LIBRARIES} ${${NAME}_LIBRARY}")
        endif()
    elseif (LP_ENCOURAGED)
        message(WARNING "${NAME} not found; compiling OpenTTD without ${NAME} is strongly disencouraged")
    endif()
endfunction()
