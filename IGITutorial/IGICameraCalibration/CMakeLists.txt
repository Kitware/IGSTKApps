project(IGICameraCalibration)

cmake_minimum_required(VERSION 2.8)

include_directories(
  ${IGICameraCalibration_SOURCE_DIR}
  ${IGICameraCalibration_BINARY_DIR}
)

#
# Use OpenCV to actually do the tracking (used by the ArUco library)
#
find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIR})

#
# Use Qt as the GUI toolkit
#
find_package(Qt4 REQUIRED)
include (${QT_USE_FILE} )

#
# Description of the GUI used by Qt Designer
#
set(IGICameraCalibration_GUI_SRCS
  MainWidget.ui
  )

#
# Resource files description used by Qt Designer
#
set(IGICameraCalibration_RCCS IGICameraCalibration.qrc)

#
# Generate rules for building source files from the resources
#
qt4_add_resources(IGICameraCalibration_RCC_SRCS ${IGICameraCalibration_RCCS})

qt4_wrap_ui(IGICameraCalibration_QT_UI_SRCS ${IGICameraCalibration_GUI_SRCS})

if(WIN32)
  set(WIN_RESOURCE_FILE IGICameraCalibration.rc)
endif(WIN32)

set(IGICameraCalibration_SRCS
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

qt4_automoc(${IGICameraCalibration_SRCS})

#
# Setup Mac OS X bundle
#
if(APPLE)
	set(ICONS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/Images")
        set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_ICON_FILE "Validation.icns")
	set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
	set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
endif()

# "WIN32" suppresses the MS-DOS command window
# "MACOSX_BUNDLE" creates a MAC application bundle
# "WIN32" and "MACOSX_BUNDLE" are defined depending on the platform
add_executable(IGICameraCalibration WIN32 MACOSX_BUNDLE ${IGICameraCalibration_QT_UI_SRCS} ${IGICameraCalibration_SRCS} ${IGICameraCalibration_RCC_SRCS} ${WIN_RESOURCE_FILE})

set(EXTRA_LIBS ${EXTRA_LIBS} ${QT_LIBRARIES} ${OpenCV_LIBS})

if(UNIX)
  if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
    SET(EXTRA_LIBS ${EXTRA_LIBS} glib-2.0)
  endif(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
endif(UNIX)

target_link_libraries(IGICameraCalibration ${EXTRA_LIBS})

#
# Install the executable (belongs in the "applications" component)
#
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "Programs"
        COMPONENT applications )

#
# Set platform specific application extension
# additionally if APPLE copy icon into Resources folder
#
if(APPLE)
  install(FILES ${MACOSX_BUNDLE_ICON}
          DESTINATION "Programs/${PROJECT_NAME}.app/Contents/Resources"
          COMPONENT applications )
  set(EXECUTABLE "${PROJECT_NAME}.app")
elseif(WIN32)
  set(EXECUTABLE "${PROJECT_NAME}.exe")
elseif(UNIX)
  if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
    set(EXECUTABLE "${PROJECT_NAME}")
  endif()
endif()

#
# Install required Qt libraries
#
if(UNIX OR WIN32)
  include(DeployQt4)
  install_qt4_executable("Programs/${EXECUTABLE}" "QtGui" "" "" "" "" "applications")
endif()

#
# Install the Readme file which has to be in the same directory as
# the executable
#
set(README_FILE READMECameraCalibration.html)
install(FILES ${README_FILE}
        DESTINATION Programs
        COMPONENT applications)
