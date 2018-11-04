set(config_file ${CMAKE_CURRENT_BINARY_DIR}/doxygen.conf)

configure_file(config/doxygen.conf.in ${config_file})

add_custom_target(
  doc
  COMMAND ${DOXYGEN} ${config_file}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )

