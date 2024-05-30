include(FetchContent)
FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glm)

add_executable(
  path_tracer
  main.cc
  ray.cc
  interval.cc
  sphere.cc
  hittable.cc
  hittable_list.cc
  camera.cc
  material.cc
  disk.cc)
target_include_directories(path_tracer PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(path_tracer PRIVATE glm::glm)
