cmake_minimum_required(VERSION 3.16)
project(MinibrowserQt)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets WebEngineWidgets)

set(SOURCES
    src/main.cpp
    src/BrowserCore.cpp
    src/MainWindow.cpp
    src/Omnibox.cpp
    src/HomepageWidget.cpp
    src/UrlResolver.cpp
    src/ThemeManager.cpp
    src/SettingsDialog.cpp          
)

set(HEADERS
    src/BrowserCore.h
    src/MainWindow.h
    src/Omnibox.h
    src/HomepageWidget.h
    src/UrlResolver.h
    src/ThemeManager.h
    src/SearchEngine.h
    src/SettingsDialog.h            
)

add_executable(minibrowser ${SOURCES} ${HEADERS})

target_link_libraries(minibrowser
    Qt6::Core
    Qt6::Widgets
    Qt6::WebEngineWidgets
)

add_custom_command(TARGET minibrowser POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:minibrowser>/resources
    COMMENT "Copiando resources (QSS) para o diretório do executável"
)