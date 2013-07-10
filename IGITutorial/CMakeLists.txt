project(IGITutorial)

cmake_minimum_required(VERSION 2.8)

#
# CPack setup to generate installers
#
set(CPACK_PACKAGE_NAME "IGITutorial")
set(CPACK_PACKAGE_VENDOR "IGSTK Developers")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IGITutorial - hands-on image-guided navigation tutorial")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "2")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "IGITutorial")

add_subdirectory(IGICameraCalibration)
add_subdirectory(IGIPivotCalibration)
add_subdirectory(IGIPlanning)
add_subdirectory(IGINavigation)
add_subdirectory(IGI3DDigitizer)
add_subdirectory(IGITrackingData)
if(WIN32)
  add_subdirectory(IGIRemoteControl)
endif(WIN32)

#
# Tell Cpack to copy the windows runtime libraries to the bin
# directory
#
if(WIN32)
  set( CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
  include(InstallRequiredSystemLibraries)
  install(FILES ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS}
        DESTINATION Programs
        COMPONENT applications)
endif(WIN32)

#
# Find Qt and check version
#
set( QT_MIN_VERSION 4.6.0 )

find_package(Qt4 REQUIRED)

set( QT_VERSION ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} )

if( ${QT_VERSION} VERSION_LESS ${QT_MIN_VERSION} )
  message( FATAL_ERROR "This application requires Qt version ${QT_MIN_VERSION} or later. Provided version is ${QT_VERSION}.")
endif()

#
# Add Qt libs in Linux manually to the executables,
# since DeployQt4 has a bug in Linux
#
if(UNIX)
  if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))

    install(FILES "${QT_LIBRARY_DIR}/libQtCore.so.${QT_VERSION}"
        DESTINATION Programs
        COMPONENT applications)

    install(FILES "${QT_LIBRARY_DIR}/libQtGui.so.${QT_VERSION}"
        DESTINATION Programs
        COMPONENT applications)

    install(FILES "${QT_LIBRARY_DIR}/libQtXml.so.${QT_VERSION}"
        DESTINATION Programs
        COMPONENT applications)

  endif(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
endif(UNIX)


#
# Install the data files (they go in the "data" component)
#
set(CT_DATA_FILE_DIR Data/CT/LegoPhantom)
set(MESH_MODELS_DATA_DIR Data/MeshModels)
set(PRINT_DATA_DIR Data/Print)
set(DATA_FOR_DEVELOPERS_DIR Data/ForDevelopers)
set(LEGO_INSTRUCTIONS Data/LegoInstructions)

file(GLOB CT_STANDARD_FILES "${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/*")
install(FILES ${CT_STANDARD_FILES}
        DESTINATION ${CT_DATA_FILE_DIR}
        COMPONENT data)

install(FILES "${CMAKE_SOURCE_DIR}/${LEGO_INSTRUCTIONS}/legoPhantomInstructions.pdf"
        DESTINATION ${PRINT_DATA_DIR}
        COMPONENT data)

install(FILES "${CMAKE_SOURCE_DIR}/Data/Calibration/ValidationGrid.pdf"
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT data)

install(FILES "${CMAKE_SOURCE_DIR}/Data/Calibration/CalibrationGrid.pdf"
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT data)

install(FILES "${CMAKE_SOURCE_DIR}/Data/TrackedMarkers/markers.pdf"
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT data)
        
file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/${MESH_MODELS_DATA_DIR}/*")
install(FILES ${MESH_FILES}
        DESTINATION ${MESH_MODELS_DATA_DIR}
        COMPONENT data)

install(FILES "${CMAKE_SOURCE_DIR}/license.txt"
        DESTINATION Programs       
        COMPONENT applications)

#
# Tutorial instructions
#
install(FILES "${CMAKE_SOURCE_DIR}/Data/Presentations/IGI-Users.pptx"
        DESTINATION Presentations       
        COMPONENT presentations)

install(FILES "${CMAKE_SOURCE_DIR}/Data/Presentations/IGI-Developers.pptx"
        DESTINATION Presentations       
        COMPONENT presentations)
		
#
# Data for developers
#
install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/FREunCorrelatedWithTREFiducials.xml"
        DESTINATION ${DATA_FOR_DEVELOPERS_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/FREunCorrelatedWithTRETarget.xml"
        DESTINATION ${DATA_FOR_DEVELOPERS_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/legoPhantomPoints.txt"
        DESTINATION ${DATA_FOR_DEVELOPERS_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/legoPhantomPointsX10.txt"
        DESTINATION ${DATA_FOR_DEVELOPERS_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/legoPhantomPointsX100.txt"
        DESTINATION ${DATA_FOR_DEVELOPERS_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/nearlyCollinearFiducials.xml"
        DESTINATION ${DATA_FOR_DEVELOPERS_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/TwoMarkerConfiguration.pdf"
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${DATA_FOR_DEVELOPERS_DIR}/VirtualKeyboard.pdf"
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT apps4Developers)

install(FILES "${CMAKE_SOURCE_DIR}/${LEGO_INSTRUCTIONS}/legoTowerInstructions.pdf"
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT apps4Developers)

#
# Text from "license.txt" is displayed in the installer
#
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license.txt")

#
# Set the icon used inside the installer wizard
#
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}\\\\IGSTK.bmp")

#
# Set the icons used for the installer and un-installer
#
if(WIN32)
  SET(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/IGI.ico")
  SET(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/IGI.ico")
endif(WIN32)

#
# Tell CPack about the components.
#
set(CPACK_COMPONENTS_ALL applications apps4Developers data presentations)

#
# Set the programs displayed under the Start menu shortcut
# We do not use CPACK_PACKAGE_EXECUTABLES because it expects the binaries to be in a
# directory named "bin" (CMake bug tracker ID: 0007829).
#
#if(WIN32)
#  set(CPACK_NSIS_MENU_LINKS "Programs\\IGICameraCalibration" "Camera Calibration" 
#                          "Programs\\IGIPivotCalibration" "Pointer Calibration"
#						  "Programs\\IGIPlanning" "Planning Software"
#						  "Programs\\IGINavigation" "Navigation Software")
#endif(WIN32)

if(WIN32)
  set(CPACK_NSIS_MENU_LINKS "Programs" "Programs"
						  "Data" "Data"
						  "Presentations" "Presentations")
endif(WIN32)

#
# More descriptive names for each of the components
#
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "applications (Calibration, Pre-Operative Planning, Navigation)")
set(CPACK_COMPONENT_APPS4DEVELOPERS_DISPLAY_NAME "apps4Developers (3D Digitizer, Tracking data logger, Remote control)")
set(CPACK_COMPONENT_DATA_DISPLAY_NAME "data (CT, LEGO building instructions)")
set(CPACK_COMPONENT_PRESENTATIONS_DISPLAY_NAME "presentations (Tutorial instructions)")

include(CPack)
