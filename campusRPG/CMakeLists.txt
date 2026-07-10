cmake_minimum_required(VERSION 3.19)
project(campusRPG LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(campusRPG
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    Character.h Character.cpp
    GameSystem.h GameSystem.cpp
    Item.h Item.cpp
    Enemy.h Enemy.cpp
    Shop.h Shop.cpp
    Quest.h Quest.cpp
    BattleSystem.h BattleSystem.cpp
    FileManager.h FileManager.cpp
    AvatarRenderer.h AvatarRenderer.cpp
    AvatarDressDialog.h AvatarDressDialog.cpp
    battlemapdialog.h battlemapdialog.cpp
    battlescenedialog.h battlescenedialog.cpp
    gamesnapshot.h
    autosaveworker.h autosaveworker.cpp
    gametimerworker.h gametimerworker.cpp
    randomeventworker.h randomeventworker.cpp
    backpackdialog.h backpackdialog.cpp
    questdialog.h questdialog.cpp
    firstaidcenterdialog.h firstaidcenterdialog.cpp

)

target_link_libraries(campusRPG
    PRIVATE
        Qt::Core
        Qt::Widgets
)
target_compile_definitions(campusRPG
    PRIVATE
        AVATAR_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets/avatar"
)

install(TARGETS campusRPG
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET campusRPG
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
