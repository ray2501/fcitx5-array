add_library(array MODULE engine.cpp array.cpp)
target_link_libraries(array Fcitx5::Core Fcitx5::Config Fcitx5::Module::QuickPhrase ${SQLite3_LIBRARIES})
target_include_directories(array PRIVATE ${PROJECT_BINARY_DIR})
set_target_properties(array PROPERTIES PREFIX "")
install(TARGETS array DESTINATION "${FCITX_INSTALL_LIBDIR}/fcitx5")

configure_file(array-addon.conf.in.in array-addon.conf.in)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/array-addon.conf.in" array-addon.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/array-addon.conf" RENAME array.conf DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon")

fcitx5_translate_desktop_file(array.conf.in array.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/array.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/inputmethod")
