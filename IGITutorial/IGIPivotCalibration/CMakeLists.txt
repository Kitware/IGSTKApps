project(PivotCalibration)
 
cmake_minimum_required(VERSION 2.8)

#
# Use IGSTK to do the pivot calibration (algorithm, transformation
# xml writing...)
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
  ${PivotCalibration_SOURCE_DIR}
  ${PivotCalibration_BINARY_DIR}
  ${IGSTK_SOURCE_DIR}/Utilities/Aruco/aruco-1.1.0_SRC
)

#
# Description of the GUI used by Qt Designer
#
set(PivotCalibration_GUI_SRCS
  PivotCalibration.ui
  )

qt4_wrap_ui(PivotCalibration_QT_UI_SRCS ${PivotCalibration_GUI_SRCS})

#
# Resource files description used by Qt Designer
#
set(PivotCalibration_RCCS PivotCalibration.qrc)

#
# Generate rules for building source files from the resources
#
qt4_add_resources(PivotCalibration_RCC_SRCS ${PivotCalibration_RCCS})

if(WIN32)
  set(WIN_RESOURCE_FILE PivotCalibration.rc)
endif(WIN32)

set(PivotCalibration_SRCS
  ../IGIConfigurationData.h
  ../IGIConfigurationData.cxx
  main.cxx
  PivotCalibration.cxx
  PivotCalibration.h
  )

qt4_automoc(${PivotCalibration_SRCS})

add_executable(IGIPivotCalibration WIN32 ${PivotCalibration_QT_UI_SRCS} ${PivotCalibration_SRCS} ${PivotCalibration_RCC_SRCS}
               ${WIN_RESOURCE_FILE})

target_link_libraries(IGIPivotCalibration IGSTK TransformReaders TransformWriters ${QT_LIBRARIES} ${OpenCV_LIBS})

#
# Install the executable (belongs in the "applications" component)
#
install(TARGETS IGIPivotCalibration
        RUNTIME
        DESTINATION Programs
        COMPONENT applications)
#
# Install the Readme file which has to be in the same directory as
# the executable
#
set(README_FILE READMEPivotCalibration.html)
install(FILES ${README_FILE}
        DESTINATION Programs
        COMPONENT applications)
