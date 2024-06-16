cmake_minimum_required(VERSION 3.16)

project(LivenessChecker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Quick)
find_package(OpenCV REQUIRED)
find_package(DLIB REQUIRED)

qt_standard_project_setup()

set(CMAKE_AUTORCC ON)

qt_add_executable(appLivenessChecker
    main.cpp
    qml_images.qrc
)

set(LIVENESS_HEADERS "set headers to liveness sdk library")
set(LIVENESS_LIBRARY "set path to pre-build liveness sdk library")

qt_add_qml_module(appLivenessChecker
    URI LivenessChecker
    VERSION 1.0
    QML_FILES Main.qml
    SOURCES webcamcapture.h webcamcapture.cpp
    SOURCES imageprovider.h imageprovider.cpp
    SOURCES livenessestimates.h livenessestimates.cpp
    RESOURCES qml_images.qrc
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appLivenessChecker PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appLivenessChecker
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_include_directories(appLivenessChecker PRIVATE ${DLIB_INCLUDE_DIRECTORY} ${LIVENESS_HEADERS})

target_link_libraries(appLivenessChecker
    PRIVATE Qt6::Quick
    ${OpenCV_LIBS}
    ${DLIB_LIBRARIES}
    ${LIVENESS_LIBRARY}
)

include(GNUInstallDirs)
install(TARGETS appLivenessChecker
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
