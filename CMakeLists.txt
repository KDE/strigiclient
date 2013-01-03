project (strigiclient)

##### cmake settings #####

cmake_minimum_required(VERSION 2.8.9 FATAL_ERROR)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(MacroFindOptionalDep)
enable_testing()


##### global variables #####
option(ENABLE_QT4 "enable Qt4 GUI" ON)
set(QT_MIN_VERSION "4.3.0")

# definition of LIB_DESTINATION which is the path where the lib is installed
include(GNUInstallDirs)
set(LIB_DESTINATION "${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}")

# Set up RPATH handling, so the libs are found if they are installed to a non-standard location.
# By default cmake builds the targets with full RPATH to everything in the build directory,
# but then removes the RPATH when installing.
# These two options below make it set the RPATH of the installed targets to all
# RPATH directories outside the current CMAKE_BINARY_DIR and also the library
# install directory. Alex
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH  TRUE)
set(CMAKE_INSTALL_RPATH                ${LIB_DESTINATION} )


##### environment inspection #####
find_optional_dep(Qt4 ENABLE_QT4 QT4_FOUND "Qt4 GUI client")
if(QT4_FOUND)
  # properly set up compile flags (QT_DEBUG/QT_NO_DEBUG, ...)
  include(${QT_USE_FILE})
endif(QT4_FOUND)

# check for required packages
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
  find_package(LibStreams REQUIRED)
  find_package(LibStreamAnalyzer REQUIRED)
  if(NOT WIN32)
    find_package(LibSearchClient REQUIRED)
  endif()
else()
  set(LIBSTREAMS_INCLUDE_DIRS
    ${libstreams_SOURCE_DIR}/include
    ${libstreams_BINARY_DIR}/include
    )
  set(LIBSTREAMANALYZER_INCLUDE_DIRS
    ${libstreamanalyzer_SOURCE_DIR}/include
    ${libstreamanalyzer_BINARY_DIR}/include
    )
    if(NOT WIN32)
      set(LIBSEARCHCLIENT_INCLUDE_DIRS
        ${strigidaemon_SOURCE_DIR}/include
        ${strigidaemon_BINARY_DIR}/include
      )
    endif()
  set(LIBSTREAMS_LIBRARIES streams)
  set(LIBSTREAMANALYZER_LIBRARIES streamanalyzer)
endif()

##### building and testing #####

# every directory needs the headers that will be installed
include_directories(${LIBSTREAMS_INCLUDE_DIRS}
	${LIBSTREAMANALYZER_INCLUDE_DIRS}
	lib/searchclient/qtdbus
)

if(NOT WIN32)
  include_directories(${LIBSEARCHCLIENT_INCLUDE_DIRS}
                      lib/htmlgui
                      lib/searchclient/filterwidget
  )
endif()

link_directories(${LIBSTREAMANALYZER_LIBRARY_DIRS})

if(NOT WIN32)
  link_directories(${LIBSEARCHCLIENT_LIBRARY_DIRS})
endif()

add_subdirectory(lib)
add_subdirectory(bin)


##### installing #####

