project(RemoteControl)
 
cmake_minimum_required(VERSION 2.8)

#
# We require IGSTK version IGSTK_MIN_VERSION or later (ArucoTracker
# was introduced in 5.0.0
#
set( IGSTK_MIN_VERSION 5.0.0 )

find_package(IGSTK REQUIRED)

set( IGSTK_VERSION ${IGSTK_VERSION_MAJOR}.${IGSTK_VERSION_MINOR}.${IGSTK_VERSION_PATCH} )

if( ${IGSTK_VERSION} VERSION_LESS ${IGSTK_MIN_VERSION} )
  message( FATAL_ERROR "This application requires IGSTK version ${IGSTK_MIN_VERSION} or later. Provided version is ${IGSTK_VERSION}.")
else()
  include(${IGSTK_USE_FILE} )
  include_directories(
  ${IGSTK_SOURCE_DIR}/Examples/TransformWriters
  ${IGSTK_SOURCE_DIR}/Examples/TransformReaders
  ${IGSTK_SOURCE_DIR}/Examples/Tools
  ${IGSTK_SOURCE_DIR}/Utilities/Aruco/aruco-1.1.0_SRC
  )
endif()
 
#
# Use Qt as the GUI toolkit
#
find_package(Qt4 REQUIRED)
include (${QT_USE_FILE} )

#
# Use OpenCV to actually do the tracking (used by the ArUco library)
#
find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIR})

include_directories(
  ${IGSTK_SOURCE_DIR}/Examples/TransformWriters
  ${IGSTK_SOURCE_DIR}/Examples/TransformReaders
  ${RemoteControl_SOURCE_DIR}
  ${RemoteControl_BINARY_DIR}
  ${IGSTK_SOURCE_DIR}/Utilities/Aruco/aruco-1.1.0_SRC
)

#
# Description of the GUI used by Qt Designer
#
set(RemoteControl_GUI_SRCS
  RemoteControl.ui
  )

qt4_wrap_ui(RemoteControl_QT_UI_SRCS ${RemoteControl_GUI_SRCS})

#
# Resource files description used by Qt Designer
#
set(RemoteControl_RCCS RemoteControl.qrc)

#
# Generate rules for building source files from the resources
#
qt4_add_resources(RemoteControl_RCC_SRCS ${RemoteControl_RCCS})

if(WIN32)
  set(WIN_RESOURCE_FILE RemoteControl.rc)
endif(WIN32)

set(RemoteControl_SRCS
  ../IGIConfigurationData.h
  ../IGIConfigurationData.cxx
  main.cxx
  RemoteControl.cxx
  RemoteControl.h
  )

qt4_automoc(${RemoteControl_SRCS})

add_executable(IGIRemoteControl WIN32 ${RemoteControl_QT_UI_SRCS} ${RemoteControl_SRCS} ${RemoteControl_RCC_SRCS}
               ${WIN_RESOURCE_FILE})

target_link_libraries(IGIRemoteControl IGSTK TransformReaders TransformWriters ${QT_LIBRARIES} ${OpenCV_LIBS})

#
# Install the executable (belongs in the "apps4Developers" component)
#
install(TARGETS IGIRemoteControl
        RUNTIME
        DESTINATION Programs
        COMPONENT apps4Developers)
#
# Install the Readme file which has to be in the same directory as
# the executable
#
set(README_FILE READMERemoteControl.html)
install(FILES ${README_FILE}
        DESTINATION Programs
        COMPONENT apps4Developers)
