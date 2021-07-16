cmake_minimum_required(VERSION 3.0)

project(PortMidi VERSION 235.0)

option(BUILD_SHARED_LIBS "Build dynamic library" ON)

add_library(PortTime src/porttime/porttime.c)
target_include_directories(PortTime PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(PortTime PROPERTIES OUTPUT_NAME porttime)
set_target_properties(PortTime PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)

add_library(PortMidi
  src/portmidi/common/portmidi.c
  src/portmidi/common/pmutil.c
)
set_target_properties(PortMidi PROPERTIES OUTPUT_NAME portmidi)
set_target_properties(PortMidi PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)
target_include_directories(PortMidi PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/portmidi/common>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(PortMidi PRIVATE pm_common porttime)

# FreeBSD has a reimplementation of alsalib, so don't restrict the ALSA backend strictly to Linux.
if(UNIX AND NOT APPLE)
  find_package(ALSA REQUIRED)
  target_sources(PortTime PRIVATE src/porttime/ptlinux.c)
  target_sources(PortMidi PRIVATE
    src/portmidi/linux/pmlinux.c
    src/portmidi/linux/pmlinuxalsa.c
    src/portmidi/linux/finddefault.c)
  target_link_libraries(PortMidi PRIVATE ALSA::ALSA)
elseif(APPLE)
  find_library(COREAUDIO_LIBRARY CoreAudio REQUIRED)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
  find_library(COREMIDI_LIBRARY CoreMidi REQUIRED)
  find_library(CORESERVICES_LIBRARY CoreServices REQUIRED)

  target_sources(PortTime PRIVATE src/porttime/ptmacosx_mach.c)
  target_link_libraries(PortTime PRIVATE ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY})

  target_sources(PortMidi PRIVATE
    src/portmidi/mac/pmmac.c
    src/portmidi/mac/pmmacosxcm.c
    src/portmidi/mac/finddefault.c
    src/portmidi/mac/readbinaryplist.c)
  target_link_libraries(PortMidi PRIVATE ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREMIDI_LIBRARY} ${CORESERVICES_LIBRARY})
elseif(WIN32)
  target_sources(PortTime PRIVATE src/porttime/ptwinmm.c)
  target_link_libraries(PortTime PRIVATE winmm)
  target_sources(PortMidi PRIVATE
    src/portmidi/windows/pmwin.c
    src/portmidi/windows/pmwinmm.c)
  target_link_libraries(PortMidi PRIVATE winmm)
else()
  message(FATAL_ERROR "Operating system not supported.")
endif()

target_link_libraries(PortMidi PRIVATE PortTime)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Library
install(TARGETS PortMidi PortTime
  EXPORT PortMidiTargets
  ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

# Headers
install(FILES
  include/pmutil.h
  include/portmidi.h
  include/porttime.h
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

# pkgconfig
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/packaging/portmidi.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/packaging/portmidi.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/packaging/portmidi.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# CMake config
set(PORTMIDI_INSTALL_CMAKEDIR "lib/cmake/PortMidi")
install(
  EXPORT PortMidiTargets
  FILE PortMidiTargets.cmake
  NAMESPACE PortMidi::
  DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
configure_package_config_file(packaging/PortMidiConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfig.cmake"
  INSTALL_DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfigVersion.cmake"
  VERSION "${CMAKE_PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/packaging/PortMidiConfigVersion.cmake"
  DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)

option(BUILD_TESTING "Include test projects" OFF)
if(BUILD_TESTING)
  add_subdirectory(test)
endif()
