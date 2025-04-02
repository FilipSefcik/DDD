add_executable(
    main
    main.cpp
    modules/module.hpp
    modules/module.cpp
    modules/module_info.hpp
    modules/module_info.cpp
    modules/pla_function.hpp
    modules/pla_function.cpp
    processing/module_manager.hpp
    processing/module_manager.cpp
    processing/mpi_manager.hpp
    processing/mpi_manager.cpp
    processing/process.hpp
    processing/process.cpp
    utils/callbacks.hpp
    utils/callbacks.cpp
    utils/mpi_communicator.hpp
    utils/mpi_communicator.cpp
    ddd_core/ddd.hpp
    ddd_core/ddd.cpp
)

# find_package(TeDDy REQUIRED)

target_link_libraries(
    main PRIVATE teddy::teddy
)

# MPI
find_package(MPI REQUIRED)
target_link_libraries(
    main PUBLIC MPI::MPI_CXX
)

target_compile_options(main PRIVATE "-Wall" "-Wextra" "-Wpedantic")
