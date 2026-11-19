include(FetchContent)

# wcore
if(NOT TARGET wcore::wcore)
	set(WCORE_BUILD_TESTS OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(wcore
		GIT_REPOSITORY https://github.com/descreetStudios/wcore.git
		GIT_TAG main
		GIT_SHALLOW TRUE)
	FetchContent_MakeAvailable(wcore)
endif()

# GLFW
if(WPLATFORM_USE_SYSTEM_GLFW)
	find_package(glfw3 CONFIG REQUIRED)
elseif(NOT TARGET glfw)
	set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
	set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
	set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

	FetchContent_Declare(glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		GIT_TAG        3.5.1
		GIT_SHALLOW    TRUE)
	FetchContent_MakeAvailable(glfw)
endif()

# eventpp
if(WPLATFORM_USE_SYSTEM_EVENTPP)
	find_package(eventpp CONFIG REQUIRED)
elseif(NOT TARGET eventpp::eventpp)
	FetchContent_Declare(eventpp
		GIT_REPOSITORY https://github.com/wqking/eventpp.git
		GIT_TAG        v0.1.3
		GIT_SHALLOW    TRUE)
	FetchContent_MakeAvailable(eventpp)
endif()
