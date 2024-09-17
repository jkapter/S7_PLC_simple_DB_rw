set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
        snap7/snap7.h
        snap7/snap7.cpp
        plc_data_handler.h
        plc_data_handler.cpp
        hintinputdialog.h
        hintinputdialog.cpp
)

set(app_icon_resource_windows "img/PLCRW.rc")

qt_add_executable(S7_DB_values_rw
	${PROJECT_SOURCES}

        ${app_icon_resource_windows}
        resrc.qrc
	)

target_link_directories(S7_DB_values_rw PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/snap7/)
target_link_libraries(S7_DB_values_rw PRIVATE Qt6::Widgets)
target_link_libraries(S7_DB_values_rw PUBLIC Snap7)

set_target_properties(S7_DB_values_rw PROPERTIES
	WIN64_EXECUTABLE ON
	WIN32_EXECUTABLE ON)
	
set(CMAKE_CXX_STANDARD_LIBRARIES "-static-libgcc -static-libstdc++ -lwsock32 -lws2_32 ${CMAKE_CSS_STANDARD_LIBRARIES}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-L${CMAKE_CURRENT_SOURCE_DIR}/lib/ -Wl,-Bstatic,--whole-archive -lwinpthread-rc -Wl,--no-whole-archive -Wl,-Bdynamic -lsnap7")
