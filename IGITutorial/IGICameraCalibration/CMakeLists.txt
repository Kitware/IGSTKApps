project(CameraCalibration)

cmake_minimum_required(VERSION 2.8)

#
# Use Qt as the GUI toolkit
#
find_package(Qt4 REQUIRED)
include (${QT_USE_FILE} )

#
# Use OpenCV to actually do the tracking (used by the ArUco library)
#
find_package(OpenCV REQUIRED)
#include_directories(${OPENCV_INCLUDE_DIR})

include_directories(
  ${CameraCalibration_SOURCE_DIR}
  ${CameraCalibration_BINARY_DIR}
)

#
# Description of the GUI used by Qt Designer
#
set(CameraCalibration_GUI_SRCS
  MainWidget.ui
  )

#
# Resource files description used by Qt Designer
#
set(CameraCalibration_RCCS CameraCalibration.qrc)

#
# Generate rules for building source files from the resources
#
qt4_add_resources(CameraCalibration_RCC_SRCS ${CameraCalibration_RCCS})

qt4_wrap_ui(CameraCalibration_QT_UI_SRCS ${CameraCalibration_GUI_SRCS})

if(WIN32)
  set(WIN_RESOURCE_FILE CameraCalibration.rc)
endif(WIN32)

set(CameraCalibration_SRCS
  ../IGIConfigurationData.h
  ../IGIConfigurationData.cxx
  main.cxx
  MainWidget.h
  MainWidget.cxx
  ImageCapture.h
  ImageCapture.cxx
  CameraCalibration.h
  CameraCalibration.cxx
  ValidateCameraCalibration.h
  ValidateCameraCalibration.cxx
  )

qt4_automoc(${CameraCalibration_SRCS})

add_executable(IGICameraCalibration WIN32 ${CameraCalibration_QT_UI_SRCS} ${CameraCalibration_SRCS} ${CameraCalibration_RCC_SRCS} ${WIN_RESOURCE_FILE})

SET(EXTRA_LIBS ${EXTRA_LIBS} ${QT_LIBRARIES} ${OpenCV_LIBS})

if(UNIX)
  if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
    SET(EXTRA_LIBS ${EXTRA_LIBS} glib-2.0)
  endif(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
endif(UNIX)

target_link_libraries(IGICameraCalibration ${EXTRA_LIBS})

#
# Install the executable (belongs in the "applications" component)
#
install(TARGETS IGICameraCalibration
        RUNTIME
        DESTINATION Programs
        COMPONENT applications)
#
# Install the Readme file which has to be in the same directory as
# the executable
#
set(README_FILE READMECameraCalibration.html)
install(FILES ${README_FILE}
        DESTINATION Programs
        COMPONENT applications)
