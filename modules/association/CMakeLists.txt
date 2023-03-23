set(FULLWIDTH_SOURCES
    association.cpp
)
add_library(association MODULE ${FULLWIDTH_SOURCES})
target_link_libraries(association Fcitx5::Core Fcitx5::Config Fcitx5::Module::Notifications)
install(TARGETS association DESTINATION "${CMAKE_INSTALL_LIBDIR}/fcitx5")

configure_file(association.conf.in.in association.conf.in)
fcitx5_translate_desktop_file("${CMAKE_CURRENT_BINARY_DIR}/association.conf.in" association.conf)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/association.conf" DESTINATION "${FCITX_INSTALL_PKGDATADIR}/addon" COMPONENT config)
