cmake_minimum_required(VERSION 3.16)
project(foundry-welcome VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick)

qt_add_executable(foundry-welcome src/main.cpp)
target_link_libraries(foundry-welcome PRIVATE Qt6::Core Qt6::Gui Qt6::Qml Qt6::Quick)

install(TARGETS foundry-welcome RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY src/qml/ DESTINATION ${CMAKE_INSTALL_DATADIR}/foundry-welcome/qml)
install(FILES data/foundry-welcome.desktop DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
install(FILES data/autostart.desktop DESTINATION /etc/xdg/autostart
        RENAME foundry-welcome.desktop)
install(FILES data/foundry-linux.png
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/256x256/apps)
