project(IGIPlanning)
  
cmake_minimum_required(VERSION 2.8)

include_directories(
  ${IGIPlanning_SOURCE_DIR}
  ${IGIPlanning_BINARY_DIR}
)

#
# Use OpenCV to actually do the tracking (used by the ArUco library)
#
find_package(OpenCV REQUIRED)
include_directories(${OPENCV_INCLUDE_DIR})

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
  ${IGSTK_SOURCE_DIR}/Examples/Tools
  ${IGSTK_SOURCE_DIR}/Utilities/Aruco/aruco-1.1.0_SRC
  )
endif()

#
# Use Qt as the GUI toolkit
# 
find_package(Qt4 REQUIRED)
set(QT_USE_QTXML 1)
include (${QT_USE_FILE} )

#
# Description of the GUI used by Qt Designer
#
set(IGIPlanning_GUI_SRCS
  IGIPlanningGUI.ui
  )

#
# Resource files description used by Qt Designer
#
set(IGIPlanning_RCCS IGIPlanning.qrc)

#
# Generate rules for building source files from the resources
#
qt4_add_resources(IGIPlanning_RCC_SRCS ${IGIPlanning_RCCS})

qt4_wrap_ui(IGIPlanning_QT_UI_SRCS ${IGIPlanning_GUI_SRCS})

set(IGIPlanning_MOC_HEADERS
  IGIPlanningGUI.h
  ButtonSetWidget.h
  ctkRangeSlider.h)

if(WIN32)
  set(WIN_RESOURCE_FILE IGIPlanning.rc)
endif(WIN32)

set(IGIPlanning_SRCS
  ../IGIConfigurationData.h
  ../IGIConfigurationData.cxx
  ErrorManager.h
  IGIPlanningGUI.h
  main.cxx
  ErrorManager.cxx
  IGIPlanningGUI.cxx
  XMLSettings.h
  FiducialSet.h
  FiducialSet.cxx
  ButtonSetWidget.h
  ButtonSetWidget.cxx
  ctkRangeSlider.h
  ctkRangeSlider.cxx
  )

qt4_wrap_cpp(IGIPlanning_MOC_SRCS ${IGIPlanning_MOC_HEADERS})

#
# Setup Mac OS X bundle
#
if(APPLE)
	set(ICONS_DIR "${${PROJECT_NAME}_SOURCE_DIR}/Images")
    set(MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_NAME} ${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_SHORT_VERSION_STRING "${CPACK_PACKAGE_VERSION}")
	set(MACOSX_BUNDLE_ICON_FILE "icon.icns")
	set(MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}")
	set(MACOSX_BUNDLE_ICON "${ICONS_DIR}/${MACOSX_BUNDLE_ICON_FILE}")
endif()

# "WIN32" suppresses the MS-DOS command window
# "MACOSX_BUNDLE" creates a MAC application bundle
# "WIN32" and "MACOSX_BUNDLE" are defined depending on the platform
add_executable(IGIPlanning WIN32 MACOSX_BUNDLE
				${IGIPlanning_QT_UI_SRCS} ${IGIPlanning_RCC_SRCS} ${IGIPlanning_SRCS}
                ${IGIPlanning_MOC_SRCS} ${WIN_RESOURCE_FILE})

target_link_libraries(IGIPlanning Tools IGSTK ${QT_LIBRARIES} )

#
# Install the executable (belongs in the "applications" component)
#
install(TARGETS ${PROJECT_NAME}
        DESTINATION "Programs"
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
set(README_FILE READMEPlanning.html)
install(FILES ${README_FILE}
        DESTINATION Programs
        COMPONENT applications)




