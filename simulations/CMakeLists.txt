# Get all immediate subdirectories of a specific folder
file(GLOB SUBDIRECTORIES LIST_DIRECTORIES true "${CMAKE_CURRENT_SOURCE_DIR}/*")

# Filter to keep only directories
set(SUBDIRS "")
foreach(subdir ${SUBDIRECTORIES})
    if(IS_DIRECTORY "${subdir}")
      add_subdirectory(${subdir})
    endif()
endforeach()
