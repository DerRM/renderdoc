cmake_minimum_required(VERSION 3.3)

project(vitahook)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wl,-q -Wall -O3 -std=c++11 -fno-rtti -fno-exceptions")

add_executable(gxm_inject
    ${CMAKE_CURRENT_LIST_DIR}/src/inject/main.c
    ${CMAKE_CURRENT_LIST_DIR}/src/inject/util.c
    ${CMAKE_CURRENT_LIST_DIR}/src/network.c
    ${CMAKE_CURRENT_LIST_DIR}/src/serializer.c
)


add_executable(vitahook_kernel
  src/kernel/main.c
  src/kernel/api.c
)

target_link_libraries(vitahook_kernel
  c
  taihenForKernel_stub
  taihenModuleUtils_stub
  SceAppMgrForDriver_stub 
  SceSysmemForDriver_stub
  SceDebugForDriver_stub
  SceProcessmgrForDriver_stub
  SceThreadmgrForDriver_stub
  #SceProcessmgrForKernel_stub
  SceSysrootForKernel_stub
  SceModulemgrForDriver_stub
  SceIofilemgrForDriver_stub
)

set_target_properties(vitahook_kernel PROPERTIES LINK_FLAGS "-nostdlib")

vita_create_self(vitahook.skprx ${CMAKE_BINARY_DIR}/bin/vitahook_kernel UNSAFE CONFIG src/kernel/exports.yml)
vita_create_stubs(kernel-stubs ${CMAKE_BINARY_DIR}/bin/vitahook_kernel ${CMAKE_CURRENT_SOURCE_DIR}/src/kernel/exports.yml KERNEL)


target_link_libraries(gxm_inject
  c
  taihen_stub
  SceLibKernel_stub
  SceSysmem_stub
  SceProcessmgr_stub
  SceKernelThreadMgr_stub
  SceKernelModuleMgr_stub
  SceIofilemgr_stub
  SceRtc_stub
  SceNetCtl_stub_weak
  SceNet_stub_weak
  SceSysmodule_stub
  SceAppMgr_stub
  gcc
  ${CMAKE_BINARY_DIR}/renderdocvita/kernel-stubs/libvita_hook_kernel_stub_weak.a
)

add_dependencies(gxm_inject kernel-stubs)

set_target_properties(gxm_inject PROPERTIES LINK_FLAGS "-nostdlib")

vita_create_self(gxm_inject.suprx ${CMAKE_BINARY_DIR}/bin/gxm_inject CONFIG ${CMAKE_CURRENT_LIST_DIR}/src/inject/exports.yml)

add_dependencies(gxm_inject.suprx vitahook.skprx)

add_custom_target(suprxsend
	COMMAND curl -T gxm_inject.suprx ftp://192.168.178.49:1337/ur0:/tai/
	DEPENDS gxm_inject.suprx
)

add_executable(vitahook
    src/main.c
    src/sfo.c
    src/network.c
    src/serializer.c
)

target_link_libraries(vitahook
  c
  taihen_stub
  SceLibKernel_stub
  SceSysmem_stub
  SceProcessmgr_stub
  SceKernelThreadMgr_stub
  SceIofilemgr_stub
  SceRtc_stub
  SceNetCtl_stub_weak
  SceNet_stub_weak
  SceSysmodule_stub
  SceAppMgr_stub
  gcc
  ${CMAKE_BINARY_DIR}/renderdocvita/kernel-stubs/libvita_hook_kernel_stub_weak.a
)

set_target_properties(vitahook PROPERTIES LINK_FLAGS "-nostdlib")

vita_create_self(vitahook.suprx ${CMAKE_BINARY_DIR}/bin/vitahook CONFIG exports.yml)

add_dependencies(vitahook kernel-stubs vitahook_kernel vitahook.skprx)
