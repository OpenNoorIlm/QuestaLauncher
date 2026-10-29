cmake_minimum_required(VERSION 3.16)
# ── Name change note ──────────────────────────────────────────────────────────
# The project was renamed from QuestaLauncher to MultiHDLauncher.
# Release archives keep their original titles for archival reference.
# ─────────────────────────────────────────────────────────────────────────────
project(MultiHDLauncher VERSION 1.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

qt_add_executable(MultiHDLauncher
    main.cpp
    mainwindow.h
    mainwindow.cpp
    prioritydialog.h
    prioritydialog.cpp
    configdialog.h
    configdialog.cpp
    excludedialog.h
    excludedialog.cpp
    librarymanager.h
    librarymanager.cpp
    librarydialog.h
    librarydialog.cpp
    gowincstdialog.h
    gowincstdialog.cpp
    resources.qrc
)

target_link_libraries(MultiHDLauncher PRIVATE Qt6::Core Qt6::Widgets Qt6::Svg)

# ───────────────────────────────────────────────────────────────────────────────
# Windows: embed icon + version info via app.rc (GUI subsystem is implied by
# qt_add_executable, but we set it explicitly for older toolchains).
# ───────────────────────────────────────────────────────────────────────────────
if(WIN32)
    set_target_properties(MultiHDLauncher PROPERTIES WIN32_EXECUTABLE TRUE)

    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/multihd_launcher.ico")
        target_sources(MultiHDLauncher PRIVATE app.rc)
        message(STATUS "[MultiHDLauncher] Windows resource app.rc included")
    else()
        message(WARNING
            "[MultiHDLauncher] multihd_launcher.ico not found — Windows icon/version "
            "resource will be skipped.\n"
            "  Convert the SVG to a multi‑size .ico, e.g.:\n"
            "    inkscape multihd_launcher.svg --export-type=ico \\\n"
            "        --export-filename=multihd_launcher.ico\n"
            "  or:\n"
            "    magick multihd_launcher.svg -define icon:auto-resize=256,128,64,48,32,16 \\\n"
            "        multihd_launcher.ico")
    endif()
endif()

# ───────────────────────────────────────────────────────────────────────────────
# Only produce packages in Release builds — skip for Debug so Qt Creator's
# day-to-day edit/compile loop stays fast.
# ───────────────────────────────────────────────────────────────────────────────
if(NOT CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "[MultiHDLauncher] Non-Release build — skipping .deb / AppImage.")
    return()
endif()

# ───────────────────────────────────────────────────────────────────────────────
# Output directories
# ───────────────────────────────────────────────────────────────────────────────
set(PKG_DIR      "${CMAKE_BINARY_DIR}/packages")
set(DEB_DIR      "${PKG_DIR}/deb")
set(APPIMAGE_DIR "${PKG_DIR}/appimage")
set(APPIMAGE_STAGING "${APPIMAGE_DIR}/staging")

file(MAKE_DIRECTORY "${DEB_DIR}")
file(MAKE_DIRECTORY "${APPIMAGE_DIR}")
file(MAKE_DIRECTORY "${APPIMAGE_STAGING}")

# ───────────────────────────────────────────────────────────────────────────────
# Install rules  (used by both CPack/deb AND the AppImage staging step)
# ───────────────────────────────────────────────────────────────────────────────
install(TARGETS MultiHDLauncher RUNTIME DESTINATION bin)
install(FILES multihd_launcher.svg
        DESTINATION share/icons/hicolor/scalable/apps/)
install(FILES MultiHDLauncher.desktop
        DESTINATION share/applications/)

# ───────────────────────────────────────────────────────────────────────────────
# .deb via CPack — runs as a post-build custom command so Qt Creator's
# Build button produces the .deb automatically.
# ───────────────────────────────────────────────────────────────────────────────
set(CPACK_PACKAGE_NAME              "MultiHDLauncher")
set(CPACK_PACKAGE_VERSION           "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR            "OpenNoorIlm")
set(CPACK_PACKAGE_CONTACT           "OpenNoorIlm <solo@opennoorilm.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qt6 multi-tool launcher: Questa FSE, Quartus Prime, Gowin EDA")
set(CPACK_PACKAGE_DESCRIPTION
    "MultiHDLauncher (formerly QuestaLauncher) is a Qt6 GUI that generates\n"
    "project files and launches Questa Intel FPGA FSE, Quartus Prime, and\n"
    "Gowin EDA from a chosen SystemVerilog project folder.")
set(CPACK_RESOURCE_FILE_LICENSE     "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_GENERATOR                 "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenNoorIlm <solo@opennoorilm.com>")
set(CPACK_DEBIAN_FILE_NAME          DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION    "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS  ON)
set(CPACK_PACKAGE_DIRECTORY         "${DEB_DIR}")
include(CPack)

# Derive the exact .deb filename CPack will produce
string(TOLOWER "${CPACK_PACKAGE_NAME}" _deb_pkg_lower)
set(DEB_FILE "${DEB_DIR}/${_deb_pkg_lower}_${PROJECT_VERSION}_amd64.deb")

# Post-build command: run cpack to generate the .deb
# (FIX: previously an extra "add_custom_command(" was nested here, leaving
#  the outer call unclosed → CMake parse error at end of file.)
add_custom_command(
    TARGET MultiHDLauncher POST_BUILD
    COMMAND ${CMAKE_CPACK_COMMAND} --config "${CMAKE_BINARY_DIR}/CPackConfig.cmake"
                                   -G DEB -B "${DEB_DIR}"
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
    COMMENT "Packaging .deb → ${DEB_FILE}"
    VERBATIM
)

# ───────────────────────────────────────────────────────────────────────────────
# linuxdeploy + linuxdeploy-plugin-qt — auto-download if not found
# (replaces the now-unmaintained linuxdeployqt)
# ───────────────────────────────────────────────────────────────────────────────
set(LD_LOCAL          "${CMAKE_BINARY_DIR}/linuxdeploy")
set(LD_QT_PLUGIN_LOCAL "${CMAKE_BINARY_DIR}/linuxdeploy-plugin-qt")

find_program(LINUXDEPLOY_EXECUTABLE NAMES linuxdeploy
             HINTS "${CMAKE_BINARY_DIR}" "$ENV{HOME}/.local/bin" "/usr/local/bin")

find_program(LINUXDEPLOY_PLUGIN_QT_EXECUTABLE NAMES linuxdeploy-plugin-qt
             HINTS "${CMAKE_BINARY_DIR}" "$ENV{HOME}/.local/bin" "/usr/local/bin")

if(NOT LINUXDEPLOY_EXECUTABLE)
    message(STATUS "[MultiHDLauncher] linuxdeploy not found — will download at build time.")
    set(LD_URL
        "https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage")
    if(NOT EXISTS "${LD_LOCAL}")
        message(STATUS "[MultiHDLauncher] Downloading linuxdeploy …")
        file(DOWNLOAD "${LD_URL}" "${LD_LOCAL}" SHOW_PROGRESS STATUS _dl_status)
        list(GET _dl_status 0 _dl_code)
        if(_dl_code EQUAL 0)
            file(CHMOD "${LD_LOCAL}" PERMISSIONS
                 OWNER_READ OWNER_WRITE OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
            message(STATUS "[MultiHDLauncher] linuxdeploy downloaded to ${LD_LOCAL}")
        else()
            list(GET _dl_status 1 _dl_msg)
            message(WARNING "[MultiHDLauncher] linuxdeploy download failed: ${_dl_msg}")
        endif()
    endif()
    if(EXISTS "${LD_LOCAL}")
        set(LINUXDEPLOY_EXECUTABLE "${LD_LOCAL}")
    endif()
endif()

if(NOT LINUXDEPLOY_PLUGIN_QT_EXECUTABLE)
    message(STATUS "[MultiHDLauncher] linuxdeploy-plugin-qt not found — will download at build time.")
    set(LD_QT_URL
        "https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage")
    if(NOT EXISTS "${LD_QT_PLUGIN_LOCAL}")
        message(STATUS "[MultiHDLauncher] Downloading linuxdeploy-plugin-qt …")
        file(DOWNLOAD "${LD_QT_URL}" "${LD_QT_PLUGIN_LOCAL}" SHOW_PROGRESS STATUS _dl_status)
        list(GET _dl_status 0 _dl_code)
        if(_dl_code EQUAL 0)
            file(CHMOD "${LD_QT_PLUGIN_LOCAL}" PERMISSIONS
                 OWNER_READ OWNER_WRITE OWNER_EXECUTE
                 GROUP_READ GROUP_EXECUTE
                 WORLD_READ WORLD_EXECUTE)
            message(STATUS "[MultiHDLauncher] linuxdeploy-plugin-qt downloaded to ${LD_QT_PLUGIN_LOCAL}")
        else()
            list(GET _dl_status 1 _dl_msg)
            message(WARNING "[MultiHDLauncher] linuxdeploy-plugin-qt download failed: ${_dl_msg}")
        endif()
    endif()
    if(EXISTS "${LD_QT_PLUGIN_LOCAL}")
        set(LINUXDEPLOY_PLUGIN_QT_EXECUTABLE "${LD_QT_PLUGIN_LOCAL}")
    endif()
endif()

# ───────────────────────────────────────────────────────────────────────────────
# AppImage — built as a second POST_BUILD step on MultiHDLauncher (after .deb)
# ───────────────────────────────────────────────────────────────────────────────
if(LINUXDEPLOY_EXECUTABLE AND LINUXDEPLOY_PLUGIN_QT_EXECUTABLE)
    # Resolve qmake from the Qt6 import
    get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    if(NOT QMAKE_EXECUTABLE)
        find_program(QMAKE_EXECUTABLE NAMES qmake6 qmake
                     HINTS "${Qt6_DIR}/../../../bin")
    endif()

    # Pre-resolve the directory prefixes so the generated script has no $(dirname)
    get_filename_component(LD_DIR          "${LINUXDEPLOY_EXECUTABLE}"          DIRECTORY)
    get_filename_component(LD_QT_PLUGIN_DIR "${LINUXDEPLOY_PLUGIN_QT_EXECUTABLE}" DIRECTORY)
    get_filename_component(QMAKE_DIR       "${QMAKE_EXECUTABLE}"                DIRECTORY)

    set(APPIMAGE_SCRIPT "${CMAKE_BINARY_DIR}/make_appimage.sh")
    file(GENERATE OUTPUT "${APPIMAGE_SCRIPT}" CONTENT
"#!/bin/sh
set -e

STAGING=\"${APPIMAGE_STAGING}\"
OUTDIR=\"${APPIMAGE_DIR}\"

# Clean slate
rm -rf \"$STAGING\"
mkdir -p \"$STAGING/usr/bin\"
mkdir -p \"$STAGING/usr/share/applications\"
mkdir -p \"$STAGING/usr/share/icons/hicolor/scalable/apps\"

# Populate FHS layout
cp \"$<TARGET_FILE:MultiHDLauncher>\"                 \"$STAGING/usr/bin/MultiHDLauncher\"
cp \"${CMAKE_SOURCE_DIR}/MultiHDLauncher.desktop\"     \"$STAGING/usr/share/applications/MultiHDLauncher.desktop\"
cp \"${CMAKE_SOURCE_DIR}/multihd_launcher.svg\"        \"$STAGING/usr/share/icons/hicolor/scalable/apps/multihd_launcher.svg\"

# Self-heal the Desktop Entry Version= key (must be a spec version, not the app version)
sed -i 's/^[[:space:]]*Version[[:space:]]*=.*/Version=1.5/' \\
       \"$STAGING/usr/share/applications/MultiHDLauncher.desktop\"

# Make sure linuxdeploy finds the qt plugin and qmake
export QMAKE=\"${QMAKE_EXECUTABLE}\"
export PATH=\"${LD_QT_PLUGIN_DIR}:${LD_DIR}:${QMAKE_DIR}:$PATH\"

# linuxdeploy writes the AppImage into the current working directory
cd \"$OUTDIR\"
\"${LINUXDEPLOY_EXECUTABLE}\" --appdir \"$STAGING\" --plugin qt --output appimage

echo \"AppImage ready in $OUTDIR/\"
"
    )

    add_custom_command(
        TARGET MultiHDLauncher POST_BUILD
        COMMAND chmod +x "${APPIMAGE_SCRIPT}"
        COMMAND "${APPIMAGE_SCRIPT}"
        WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
        COMMENT "Packaging AppImage with linuxdeploy → ${APPIMAGE_DIR}/"
        VERBATIM
    )

    message(STATUS "[MultiHDLauncher] AppImage will be built automatically")
    message(STATUS "  linuxdeploy           : ${LINUXDEPLOY_EXECUTABLE}")
    message(STATUS "  linuxdeploy-plugin-qt : ${LINUXDEPLOY_PLUGIN_QT_EXECUTABLE}")
    message(STATUS "  qmake                 : ${QMAKE_EXECUTABLE}")
else()
    message(WARNING
        "[MultiHDLauncher] linuxdeploy / linuxdeploy-plugin-qt unavailable — AppImage skipped.\n"
        "  Install globally:\n"
        "    sudo mv linuxdeploy-x86_64.AppImage            /usr/local/bin/linuxdeploy\n"
        "    sudo mv linuxdeploy-plugin-qt-x86_64.AppImage  /usr/local/bin/linuxdeploy-plugin-qt")
endif()