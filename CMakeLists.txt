cmake_minimum_required(VERSION 3.21)
project(SCAPMS VERSION 1.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()
qt_add_executable(SCAPMS
    src/main.cpp
    src/model/patientrecord.h src/model/patientrecord.cpp
    src/model/appointable.h
    src/model/generalpatient.h src/model/generalpatient.cpp
    src/model/emergencypatient.h src/model/emergencypatient.cpp
    src/model/followuppatient.h src/model/followuppatient.cpp
    src/model/appointment.h src/model/appointment.cpp
    src/model/clinicdatabase.h src/model/clinicdatabase.cpp
    src/controller/searchengine.h src/controller/searchengine.cpp
    src/controller/clinicmanager.h src/controller/clinicmanager.cpp
    src/utils/filehandler.h src/utils/filehandler.cpp
    src/gui/mainwindow.h src/gui/mainwindow.cpp
)
target_include_directories(SCAPMS PRIVATE src)
target_link_libraries(SCAPMS PRIVATE Qt6::Widgets)
