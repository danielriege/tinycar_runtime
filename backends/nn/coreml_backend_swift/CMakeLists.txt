_swift_generate_cxx_header_target(
  coreml_backend_swift_h
  CoreMLSwift
  "${CMAKE_CURRENT_BINARY_DIR}/include/coreml_backend_swift/coreml_backend_swift.h"
  SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/coreml_backend.swift")

add_library(coreml_backend_swift STATIC coreml_backend.swift)
add_dependencies(coreml_backend_swift coreml_backend_swift_h)
target_include_directories(coreml_backend_swift PUBLIC "${CMAKE_CURRENT_BINARY_DIR}/include")
set_target_properties(coreml_backend_swift PROPERTIES Swift_MODULE_NAME "CoreMLSwift")
target_compile_options(coreml_backend_swift PUBLIC
  "$<$<COMPILE_LANGUAGE:Swift>:-cxx-interoperability-mode=default>")