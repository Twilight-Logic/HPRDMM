
INCLUDE_DIRECTORIES(
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/src
  ${CMAKE_SOURCE_DIR}/src
  ${CMAKE_SOURCE_DIR}/src/gui
  $ENV{CMAKE_INCLUDE_PATH}
  )

LINK_DIRECTORIES(${STELLARIUM_BUILD_DIR}/src)

SET(HPRDmm_SRCS
  main.cpp
  gui/HPRDmmDialog.h
  gui/HPRDmmCfg.h
  gui/HPRDmmFunc.h
  gui/HPRDmmDialog.cpp
  gui/HPRDmmCfg.cpp
  gui/HPDRmmFunc.cpp
  )


# Add here all the .ui files
SET(HPRDmm_UIS
  gui/HPRDmmDialog.ui
  gui/HPRDmmCfg.ui
  gui/HPRDmmFunc.ui
  )
QT4_WRAP_UI(HPRDmm_UIS_H ${HPRDmm_UIS})

# Compile the resource files
SET(HPRDmm_RES
  ${CMAKE_SOURCE_DIR}/resources/gpsLocator.qrc)
QT4_ADD_RESOURCES(HPRDmm_RES_CXX ${HPRDmm_RES})

# Add here all the files which require moc compilation
# that is, all the headers with SIGNAL/SLOTS/PLUGIN_INTERFACE etc..
SET(HPRDmm_MOC_HDRS
  gui/HPRDmmDialog.h
  gui/HPRDmmCfg.h
  gui/HPRDmmFunc.h
  )

# After this call, HPRDmm_MOC_SRCS = moc_HPRDmm.cxx
QT4_WRAP_CPP(HPRDmm_MOC_SRCS ${HPRDmm_MOC_HDRS})

SET(extLinkerOption ${QT_LIBRARIES} ${JPEG_LIBRARIES} ${PNG_LIBRARIES} ${OPENGL_LIBRARIES} ${ICONV_LIBRARIES} ${INTL_LIBRARIES})

############### For building the dynamic library ######################
IF(BUILD_DYNAMIC_LIB)
 ADD_LIBRARY(HPRDmm MODULE ${HPRDmm_SRCS} ${HPRDmm_MOC_SRCS} ${HPRDmm_RES_CXX} ${HPRDmm_UIS_H})

 IF(APPLE)
  FIND_LIBRARY(OPENGL_LIBRARY OpenGL)
  MARK_AS_ADVANCED(OPENGL_LIBRARY)
  SET_TARGET_PROPERTIES(HPRDmm PROPERTIES LINK_FLAGS "-undefined dynamic_lookup" SUFFIX ".dylib")
 ENDIF(APPLE)

 IF(WIN32)
  SET_TARGET_PROPERTIES(HPRDmm PROPERTIES LINK_FLAGS "-enable-runtime-pseudo-reloc -Wl,--allow-multiple-definition" )
 ELSE(WIN32)
 ENDIF(WIN32)

 TARGET_LINK_LIBRARIES(HPRDmm ${extLinkerOption})

 INSTALL(TARGETS HPRDmm DESTINATION "modules/${PACKAGE}")
ENDIF(BUILD_DYNAMIC_LIB)

############### For building the static library ######################
IF(BUILD_STATIC_LIB)
 ADD_LIBRARY(HPRDmm-static STATIC ${HPRDmm_SRCS} ${HPRDmm_MOC_SRCS} ${HPRDmm_RES_CXX} ${GpsLocator_UIS_H})
 SET_TARGET_PROPERTIES(HPRDmm-static PROPERTIES OUTPUT_NAME "HPRDmm")
 TARGET_LINK_LIBRARIES(HPRDmm-static ${StelMain} ${extLinkerOption})
 SET_TARGET_PROPERTIES(HPRDmm-static PROPERTIES COMPILE_FLAGS "-fPIC -DQT_STATICPLUGIN")
ENDIF(BUILD_STATIC_LIB)

