include(FetchContent)

FetchContent_Declare(
  glm
  GIT_REPOSITORY https://github.com/g-truc/glm.git
  GIT_TAG 1.0.1)
FetchContent_MakeAvailable(glm)

find_package(Vulkan REQUIRED)

FetchContent_Declare(
  glfw
  GIT_REPOSITORY https://github.com/glfw/glfw
  GIT_TAG 3.4)
FetchContent_MakeAvailable(glfw)

add_executable(
  cpu_tracer
  cpu_tracer.cc
  ray.cc
  interval.cc
  sphere.cc
  hittable.cc
  hittable_list.cc
  camera.cc
  material.cc
  disk.cc
  portal_material.cc)
target_include_directories(cpu_tracer PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(cpu_tracer PRIVATE glm::glm)

add_executable(gpu_tracer gpu_tracer.cc vulkan_engine.cc scene.cc)
target_include_directories(gpu_tracer PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_link_libraries(
  gpu_tracer
  PRIVATE glm::glm
  PRIVATE Vulkan::Vulkan
  PRIVATE glfw)
