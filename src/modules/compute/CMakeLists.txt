set(SRCS
	Texture.h Texture.cpp
	Types.h
	Compute.h
	Shader.h Shader.cpp
	TextureConfig.h TextureConfig.cpp
)
set(LIB compute)
find_package(OpenCL)
if (OpenCL_INCLUDE_DIRS)
	list(APPEND SRCS
		cl/CL.h
		cl/CLCompute.cpp cl/CLCompute.h
		cl/CLMapping.h
		cl/CLTypes.h
		cl/CLSymbol.c cl/CLSymbol.h
	)
	set(OPENCL_LIBRARY ${OpenCL_LIBRARY})
else()
	# if opencl wasn't found, cuda might take over
	find_package(CUDA)
	if (CUDA_FOUND)
		list(APPEND SRCS
			cuda/CUDA.h
			cuda/CUDACompute.cpp cuda/CUDACompute.h
			cuda/CUDATypes.h
		)
	endif()
endif()

if (NOT OpenCL_FOUND AND NOT OpenCL_INCLUDE_DIRS)
	list(APPEND SRCS
		dummy/DummyCompute.cpp
	)
endif()

engine_add_module(TARGET ${LIB} SRCS ${SRCS} DEPENDENCIES util)
if (OpenCL_INCLUDE_DIRS)
	target_include_directories(${LIB} PRIVATE ${OpenCL_INCLUDE_DIRS})
endif()
if (CUDA_TOOLKIT_INCLUDE)
	target_include_directories(${LIB} PRIVATE ${CUDA_TOOLKIT_INCLUDE})
endif()

if (TESTS)
	set(TEST_SRCS
		tests/ComputeShaderTest.cpp
	)

	gtest_suite_sources(tests ${TEST_SRCS})
	gtest_suite_deps(tests ${LIB})
	generate_compute_shaders(tests test)
endif()
