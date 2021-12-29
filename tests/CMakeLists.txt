add_executable(ipc_test test.cpp)
add_executable(ipc_server server.cpp)
add_executable(ipc_client client.cpp)
add_executable(ipc_client_sendrecv client_sendrecv.cpp)
set_property(TARGET ipc_test PROPERTY CXX_STANDARD 14)
add_test(ipc ipc_test)
target_link_libraries(ipc_test ipc)
target_link_libraries(ipc_server ipc)
target_link_libraries(ipc_client ipc)
target_link_libraries(ipc_client_sendrecv ipc)
target_compile_options(ipc_test
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-g>
      )
target_compile_options(ipc_server
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-g>
      )
target_compile_options(ipc_client
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-g>
      )
target_compile_options(ipc_client_sendrecv
      PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-g>
      )

