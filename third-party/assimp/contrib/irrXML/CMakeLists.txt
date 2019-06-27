set( IrrXML_SRCS
  CXMLReaderImpl.h
  heapsort.h
  irrArray.h
  irrString.h
  irrTypes.h
  irrXML.cpp
  irrXML.h
)

add_library(IrrXML STATIC ${IrrXML_SRCS})
set(IRRXML_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "IrrXML_Include" )
set(IRRXML_LIBRARY "IrrXML" CACHE INTERNAL "IrrXML" )



set_target_properties(IrrXML
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        )

install(TARGETS IrrXML
        EXPORT raisimOgre-targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${LIBLEGACY_INCLUDE_DIRS}
        )

export(
        TARGETS IrrXML
        NAMESPACE raisim::
        FILE cmake/raisimOgre-targets.cmake
)

# Install headers
install(
        DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}
        DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

add_library(assimp::IrrXML ALIAS IrrXML)