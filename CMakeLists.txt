cmake_minimum_required(VERSION 3.0)

project(PortMIDI VERSION 235.0)

option(BUILD_SHARED_LIBS "Build dynamic library" ON)

add_library(PortTime porttime/porttime.c)
target_include_directories(PortTime PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(PortTime PROPERTIES OUTPUT_NAME porttime)
set_target_properties(PortTime PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)

add_library(PortMIDI src/common/portmidi.c src/common/pmutil.c)
set_target_properties(PortMIDI PROPERTIES OUTPUT_NAME portmidi)
set_target_properties(PortMIDI PROPERTIES
  WINDOWS_EXPORT_ALL_SYMBOLS TRUE
)
target_include_directories(PortMIDI PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src/common>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(PortMIDI PRIVATE pm_common porttime)

# FreeBSD has a reimplementation of alsalib, so don't restrict the ALSA backend strictly to Linux.
if(UNIX AND NOT APPLE)
  find_package(ALSA REQUIRED)
  target_sources(PortTime PRIVATE porttime/ptlinux.c)
  target_sources(PortMIDI PRIVATE src/linux/pmlinux.c src/linux/pmlinuxalsa.c src/linux/finddefault.c)
  target_link_libraries(PortMIDI PRIVATE ALSA::ALSA)
elseif(APPLE)
  find_library(COREAUDIO_LIBRARY CoreAudio REQUIRED)
  find_library(COREFOUNDATION_LIBRARY CoreFoundation REQUIRED)
  find_library(COREMIDI_LIBRARY CoreMidi REQUIRED)
  find_library(CORESERVICES_LIBRARY CoreServices REQUIRED)

  target_sources(PortTime PRIVATE porttime/ptmacosx_mach.c)
  target_link_libraries(PortTime PRIVATE ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY})

  target_sources(PortMIDI PRIVATE src/mac/pmmac.c src/mac/pmmacosxcm.c src/mac/finddefault.c src/mac/readbinaryplist.c)
  target_link_libraries(PortMIDI PRIVATE ${COREAUDIO_LIBRARY} ${COREFOUNDATION_LIBRARY} ${COREMIDI_LIBRARY} ${CORESERVICES_LIBRARY})
elseif(WIN32)
  target_sources(PortTime PRIVATE porttime/ptwinmm.c)
  target_link_libraries(PortTime PRIVATE winmm)
  target_sources(PortMIDI PRIVATE src/windows/pmwin.c src/windows/pmwinmm.c)
  target_link_libraries(PortMIDI PRIVATE winmm)
else()
  message(FATAL_ERROR "Operating system not supported.")
endif()

target_link_libraries(PortMIDI PRIVATE PortTime)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Library
install(TARGETS PortMIDI PortTime
  EXPORT PortMIDITargets
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
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/portmidi.pc.in
  ${CMAKE_CURRENT_BINARY_DIR}/portmidi.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/portmidi.pc DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

# CMake config
set(PORTMIDI_INSTALL_CMAKEDIR "lib/cmake/PortMIDI")
install(
  EXPORT PortMIDITargets
  FILE PortMIDITargets.cmake
  NAMESPACE PortMIDI::
  DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
configure_package_config_file(PortMIDIConfig.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/PortMIDIConfig.cmake"
  INSTALL_DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/PortMIDIConfigVersion.cmake"
  VERSION "${CMAKE_PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion
)
install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/PortMIDIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/PortMIDIConfigVersion.cmake"
  DESTINATION "${PORTMIDI_INSTALL_CMAKEDIR}"
)
