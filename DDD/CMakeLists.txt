add_executable(
    main
    main.cpp
)

find_package(TeDDy REQUIRED)

target_link_libraries(
    main PRIVATE teddy::teddy
)

# MPI
find_package(MPI REQUIRED)
target_link_libraries(
    main PUBLIC MPI::MPI_CXX
)

target_compile_options(main PRIVATE "-Wall" "-Wextra" "-Wpedantic")
