project(server)
set(SRCS
	Server.cpp Server.h
)
set(LUA_SRCS
	ai/blacksmith.lua
	ai/rabbit.lua
	ai/shared.lua
	ai/wolf.lua
	behaviourtrees.lua
	attributes.lua
	event/generic.lua
	events.lua
	stock.lua
	map/map001.lua
)
set(FILES)
set(MANIFESTS
	deployment/deployment.yaml.in
	deployment/route.yaml.in
	deployment/service.yaml.in
	deployment/secrets.yaml.in
)

engine_add_executable(TARGET ${PROJECT_NAME} SRCS ${SRCS} FILES ${FILES} LUA_SRCS ${LUA_SRCS})
engine_target_link_libraries(TARGET ${PROJECT_NAME} DEPENDENCIES backend)
engine_docker(${PROJECT_NAME})
engine_manifests(${PROJECT_NAME} ${MANIFESTS})
