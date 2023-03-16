cmake_minimum_required(VERSION 3.16)
project(fcitx5-array VERSION 0.7.1)

find_package(ECM 1.0.0 REQUIRED)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})
include(FeatureSummary)
include(GNUInstallDirs)
include(ECMUninstallTarget)

find_package(Fcitx5Core REQUIRED)
find_package(Fcitx5Module REQUIRED COMPONENTS QuickPhrase Notifications)
find_package(SQLite3 REQUIRED)
find_package(Gettext REQUIRED)
find_package(fmt REQUIRED)

# Setup some compiler option that is generally useful and compatible with Fcitx 5 (C++17)
include("${FCITX_INSTALL_CMAKECONFIG_DIR}/Fcitx5Utils/Fcitx5CompilerSettings.cmake")
add_definitions(-DFCITX_GETTEXT_DOMAIN=\"fcitx5-array\")
fcitx5_add_i18n_definition()

add_subdirectory(po)
add_subdirectory(src)
add_subdirectory(data)
add_subdirectory(icon)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
