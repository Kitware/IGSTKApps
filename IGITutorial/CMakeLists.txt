project(IGITutorial)

cmake_minimum_required(VERSION 2.8)

add_subdirectory(IGICameraCalibration)
add_subdirectory(IGIPivotCalibration)
add_subdirectory(IGIPlanning)
add_subdirectory(IGINavigation)

#
#  Tell Cpack to copy the windows runtime libraries to the bin
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
# Find Qt
#
set( QT_MIN_VERSION 4.6.0 )

find_package(Qt4 REQUIRED)

set( QT_VERSION ${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}.${QT_VERSION_PATCH} )

if( ${QT_VERSION} VERSION_LESS ${QT_MIN_VERSION} )
  message( FATAL_ERROR "This application requires Qt version ${QT_MIN_VERSION} or later. Provided version is ${QT_VERSION}.")
else()
	#
	# Install the Qt .dll/.so files (they go into Programs folder)
	#
	if(WIN32)
	  install(FILES "${QT_LIBRARY_DIR}/QtCore4.dll"
			DESTINATION Programs
			COMPONENT applications)

	  install(FILES "${QT_LIBRARY_DIR}/QtGui4.dll"
			DESTINATION Programs
			COMPONENT applications)

	  install(FILES "${QT_LIBRARY_DIR}/QtXml4.dll"
			DESTINATION Programs
			COMPONENT applications)
	endif(WIN32)

	if(UNIX)
	  if(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
	    install(FILES "${QT_LIBRARY_DIR}/libQtCore.so"
			DESTINATION Programs
			COMPONENT applications)

	    install(FILES "${QT_LIBRARY_DIR}/libQtGui.so"
			DESTINATION Programs
			COMPONENT applications)

	    install(FILES "${QT_LIBRARY_DIR}/libQtXml.so"
			DESTINATION Programs
			COMPONENT applications)
	  endif(NOT (${CMAKE_SYSTEM_NAME} MATCHES "Darwin"))
	endif(UNIX)
endif()



#
# Install the data files (they go in the "data" component)
#
set(CT_DATA_FILE_DIR Data/CT/Standard)
set(CALIBRATION_DATA_DIR Data/Calibration)
set(TRACKED_MARKERS_DATA_DIR Data/TrackedMarkers)
set(MESH_MODELS_DATA_DIR Data/MeshModels)
set(PRINT_DATA_DIR Data/Print)

file(GLOB CT_STANDARD_FILES "${CMAKE_SOURCE_DIR}/${CT_DATA_FILE_DIR}/*")
install(FILES ${CT_STANDARD_FILES}
        DESTINATION ${CT_DATA_FILE_DIR}
        COMPONENT data)

install(FILES "${CMAKE_SOURCE_DIR}/Data/legoPhantomInstructions.pdf"
        DESTINATION ${PRINT_DATA_DIR}
        COMPONENT data)

file(GLOB CALIBRATION_GRID_FILES "${CMAKE_SOURCE_DIR}/${CALIBRATION_DATA_DIR}/*")
install(FILES ${CALIBRATION_GRID_FILES}
        DESTINATION ${PRINT_DATA_DIR}       
        COMPONENT data)

file(GLOB TRACKED_MARKER_FILES "${CMAKE_SOURCE_DIR}/${TRACKED_MARKERS_DATA_DIR}/*")
install(FILES ${TRACKED_MARKER_FILES}
        DESTINATION ${PRINT_DATA_DIR}
        COMPONENT data)
        
file(GLOB MESH_FILES "${CMAKE_SOURCE_DIR}/${MESH_MODELS_DATA_DIR}/*")
install(FILES ${MESH_FILES}
        DESTINATION ${MESH_MODELS_DATA_DIR}
        COMPONENT data)

#
# CPack setup to generate installers
#
set(CPACK_PACKAGE_NAME "IGITutorial")
set(CPACK_PACKAGE_VENDOR "IGSTK Developers")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "IGITutorial - hands-on image-guided navigation tutorial")
set(CPACK_PACKAGE_VERSION_MAJOR "2")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}.${CPACK_PACKAGE_VERSION_PATCH}")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "IGITutorial")

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
set(CPACK_COMPONENTS_ALL applications data)

#
# Set the programs displayed under the Start menu shortcut
# We do not use CPACK_PACKAGE_EXECUTABLES because it expects the binaries to be in a
# directory named "bin" (CMake bug tracker ID: 0007829).
#
if(WIN32)
  set(CPACK_NSIS_MENU_LINKS "Programs\\IGICameraCalibration" "Camera Calibration" 
                          "Programs\\IGIPivotCalibration" "Pointer Calibration"
						  "Programs\\IGIPlanning" "Planning Software"
						  "Programs\\IGINavigation" "Navigation Software"
						  "Data" "Tutorial Data")
endif(WIN32)

#
# More descriptive names for each of the components
#
set(CPACK_COMPONENT_APPLICATIONS_DISPLAY_NAME "applications (Calibration, Pre-Operative Planning, Navigation)")
set(CPACK_COMPONENT_DATA_DISPLAY_NAME "data (CT, LEGO building instructions)")

include(CPack)
