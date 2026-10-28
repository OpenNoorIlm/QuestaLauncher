cmake_minimum_required(VERSION 3.16)
project(QuestaLauncher VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Svg)

qt_add_executable(QuestaLauncher
    main.cpp
    mainwindow.h
    mainwindow.cpp
    prioritydialog.h
    prioritydialog.cpp
    configdialog.h
    configdialog.cpp
    excludedialog.h
    excludedialog.cpp
    resources.qrc
)

target_link_libraries(QuestaLauncher PRIVATE Qt6::Core Qt6::Widgets Qt6::Svg)

# -------------------------------------------------------------------
# Package output directories
# -------------------------------------------------------------------
set(PACKAGE_OUTPUT_DIR  "${CMAKE_BINARY_DIR}/packages")
set(DEB_OUTPUT_DIR      "${PACKAGE_OUTPUT_DIR}/deb")
set(APPIMAGE_OUTPUT_DIR "${PACKAGE_OUTPUT_DIR}/appimage")

file(MAKE_DIRECTORY "${DEB_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${APPIMAGE_OUTPUT_DIR}")

# -------------------------------------------------------------------
# Install rules (needed for .deb packaging)
# -------------------------------------------------------------------
install(TARGETS QuestaLauncher
        RUNTIME DESTINATION bin
)

install(FILES questa_launcher.svg
        DESTINATION share/icons/hicolor/scalable/apps/
)

install(FILES QuestaLauncher.desktop
        DESTINATION share/applications/
)

# -------------------------------------------------------------------
# CPack configuration for .deb
# -------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "QuestaLauncher")
set(CPACK_PACKAGE_VENDOR "OpenNoorIlm")
set(CPACK_PACKAGE_CONTACT "OpenNoorIlm <solo@opennoorilm.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Qt6 based launcher")
set(CPACK_PACKAGE_DESCRIPTION "QuestaLauncher is a Qt6-based application built with CMake.")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "OpenNoorIlm <solo@opennoorilm.com>")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

# Redirect .deb output into packages/deb/
set(CPACK_PACKAGE_DIRECTORY "${DEB_OUTPUT_DIR}")

include(CPack)

# -------------------------------------------------------------------
# AppImage generation via linuxdeployqt
# -------------------------------------------------------------------
option(BUILD_APPIMAGE "Generate AppImage using linuxdeployqt" ON)

if(BUILD_APPIMAGE)
    find_program(LINUXDEPLOYQT_EXECUTABLE linuxdeployqt)
    if(LINUXDEPLOYQT_EXECUTABLE)
        get_target_property(QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)

        add_custom_target(AppImage
            # Prepare an isolated staging directory inside packages/appimage/
            COMMAND ${CMAKE_COMMAND} -E copy
                    $<TARGET_FILE:QuestaLauncher>
                    ${APPIMAGE_OUTPUT_DIR}/QuestaLauncher
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/QuestaLauncher.desktop
                    ${APPIMAGE_OUTPUT_DIR}/QuestaLauncher.desktop
            COMMAND ${CMAKE_COMMAND} -E copy
                    ${CMAKE_SOURCE_DIR}/questa_launcher.svg
                    ${APPIMAGE_OUTPUT_DIR}/questa_launcher.svg
            # Run linuxdeployqt from the staging directory so the AppImage
            # is produced there.
            COMMAND ${LINUXDEPLOYQT_EXECUTABLE}
                    ${APPIMAGE_OUTPUT_DIR}/QuestaLauncher.desktop
                    -appimage
                    -unsupported-allow-new-glibc
                    -qmake=${QMAKE_EXECUTABLE}
            WORKING_DIRECTORY ${APPIMAGE_OUTPUT_DIR}
            DEPENDS QuestaLauncher
            COMMENT "Creating AppImage in packages/appimage/"
        )
    else()
        message(WARNING "linuxdeployqt not found. "
                        "The AppImage target will not be available. "
                        "Install it from https://github.com/probonopd/linuxdeployqt/releases")
    endif()
endif()

# -------------------------------------------------------------------
# Convenience targets
# -------------------------------------------------------------------
if(TARGET AppImage)
    add_custom_target(package_all
        COMMAND ${CMAKE_CPACK_COMMAND} -G DEB
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target AppImage
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMENT "Generating DEB and AppImage into packages/"
    )
    add_dependencies(package_all QuestaLauncher)
endif()