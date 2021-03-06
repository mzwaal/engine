set(SRCS
	aggro/AggroMgr.h aggro/AggroMgr.cpp
	aggro/Entry.h
	AI.h AI.cpp
	AIFactories.h
	AIRegistry.h AIRegistry.cpp
	common/IFactoryRegistry.h
	common/IParser.h
	common/Log.h
	common/Math.h
	common/MemoryAllocator.h
	common/MoveVector.h
	common/NonCopyable.h
	common/Random.h
	common/String.h
	common/Thread.h
	common/Types.h
	conditions/And.h
	conditions/ConditionParser.h
	conditions/False.h
	conditions/Filter.h
	conditions/HasEnemies.h
	conditions/ICondition.h
	conditions/IsCloseToGroup.h
	conditions/IsGroupLeader.h
	conditions/IsInGroup.h
	conditions/Not.h
	conditions/Or.h
	conditions/True.h
	filter/IFilter.h
	filter/SelectEmpty.h
	filter/SelectGroupLeader.h
	filter/SelectGroupMembers.h
	filter/SelectHighestAggro.h
	filter/SelectZone.h
	filter/Union.h
	filter/Intersection.h
	filter/First.h
	filter/Last.h
	filter/Random.h
	filter/Difference.h
	filter/Complement.h
	filter/SelectAll.h
	IAIFactory.h
	ICharacter.h
	group/GroupId.h
	group/GroupMgr.h group/GroupMgr.cpp
	movement/SelectionSeek.h
	movement/GroupFlee.h
	movement/GroupSeek.h
	movement/Steering.h
	movement/TargetFlee.h
	movement/TargetSeek.h
	movement/Wander.h
	movement/WeightedSteering.h
	server/AIAddNodeMessage.h
	server/AIChangeMessage.h
	server/AICharacterDetailsMessage.h
	server/AICharacterStaticMessage.h
	server/AIDeleteNodeMessage.h
	server/AINamesMessage.h
	server/AIPauseMessage.h
	server/AISelectMessage.h
	server/AIStateMessage.h
	server/AIStepMessage.h
	server/AIStubTypes.h
	server/AIUpdateNodeMessage.h
	server/AddNodeHandler.h
	server/ChangeHandler.h
	server/DeleteNodeHandler.h
	server/IProtocolHandler.h
	server/IProtocolMessage.h
	server/Network.h server/Network.cpp
	server/PauseHandler.h
	server/ProtocolHandlerRegistry.h
	server/ProtocolMessageFactory.h
	server/ResetHandler.h
	server/SelectHandler.h
	server/Server.h server/Server.cpp
	server/StepHandler.h
	server/UpdateNodeHandler.h
	zone/Zone.h zone/Zone.cpp
	SimpleAI.h
	tree/Fail.h
	tree/Limit.h
	tree/Idle.h
	tree/Invert.h
	tree/ITask.h
	tree/ITimedNode.h
	tree/loaders/ITreeLoader.h
	tree/Parallel.h
	tree/PrioritySelector.h
	tree/ProbabilitySelector.h
	tree/RandomSelector.h
	tree/Selector.h
	tree/Sequence.h
	tree/Steer.h
	tree/Succeed.h
	tree/TreeNode.h tree/TreeNode.cpp
	tree/TreeNodeParser.h
)
set(LIB ai)
engine_add_module(TARGET ${LIB} SRCS ${SRCS} DEPENDENCIES commonlua)
target_include_directories(${LIB} PUBLIC .)
target_compile_definitions(${LIB} PUBLIC -DAI_INCLUDE_LUA=1)

set(TEST_SRCS
	tests/AggroTest.cpp
	tests/GeneralTest.cpp
	tests/GroupTest.cpp
	tests/LUAAIRegistryTest.cpp
	tests/LUATreeLoaderTest.cpp
	tests/MessageTest.cpp
	tests/MovementTest.cpp
	tests/NodeTest.cpp
	tests/ParserTest.cpp
	tests/TestShared.cpp
	tests/ZoneTest.cpp
)
gtest_suite_files(tests tests/testluaregistry.lua)
gtest_suite_sources(tests ${TEST_SRCS})
gtest_suite_deps(tests ${LIB})

gtest_suite_begin(tests-${LIB} TEMPLATE ${ROOT_DIR}/src/modules/core/tests/main.cpp.in)
gtest_suite_sources(tests-${LIB} ${TEST_SRCS} ../core/tests/AbstractTest.cpp)
gtest_suite_files(tests-${LIB} tests/testluaregistry.lua)
gtest_suite_deps(tests-${LIB} ${LIB})
gtest_suite_end(tests-${LIB})
