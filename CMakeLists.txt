cmake_minimum_required(VERSION 3.16)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

project(S7_DB_values_rw VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt6 REQUIRED COMPONENTS Widgets LinguistTools)
qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES en)

add_subdirectory(src)

qt_add_translations(S7_DB_values_rw TS_FILES lng/S7_DB_values_rw.ts)
