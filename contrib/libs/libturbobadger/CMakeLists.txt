set(SRCS
	tb/animation/tb_animation_utils.h
	tb/animation/tb_animation.cpp
	tb/animation/tb_widget_animation.h
	tb/animation/tb_animation.h
	tb/animation/tb_widget_animation.cpp
	tb/utf8/utf8.cpp
	tb/utf8/utf8.h
	tb/renderers/tb_renderer_gl.cpp
	tb/renderers/tb_renderer_batcher.cpp
	tb/renderers/tb_renderer_gl.h
	tb/renderers/tb_renderer_batcher.h
	tb/thirdparty/stb_image.h
	tb/thirdparty/stb_truetype.h
	tb/parser/tb_parser.h
	tb/parser/tb_parser.cpp
	tb/image/tb_image_manager.cpp
	tb/image/tb_image_widget.cpp
	tb/image/tb_image_widget.h
	tb/image/tb_image_manager.h
	tb/tb_bitmap_fragment.cpp
	tb/tb_clipboard_sdl.cpp
	tb/tb_color.cpp
	tb/tb_core.cpp
	tb/tb_debug.cpp
	tb/tb_dimension.cpp
	tb/tb_editfield.cpp
	tb/tb_file_posix.cpp
	tb/tb_file_sdl.cpp
	tb/tb_font_renderer.cpp
	tb/tb_font_renderer_freetype.cpp
	tb/tb_font_renderer_stb.cpp
	tb/tb_font_renderer_tbbf.cpp
	tb/tb_geometry.cpp
	tb/tb_hash.cpp
	tb/tb_hashtable.cpp
	tb/tb_id.cpp
	tb/tb_image_loader_stb.cpp
	tb/tb_inline_select.cpp
	tb/tb_language.cpp
	tb/tb_layout.cpp
	tb/tb_linklist.cpp
	tb/tb_list.cpp
	tb/tb_menu_window.cpp
	tb/tb_message_window.cpp
	tb/tb_msg.cpp
	tb/tb_node_ref_tree.cpp
	tb/tb_node_tree.cpp
	tb/tb_object.cpp
	tb/tb_popup_window.cpp
	tb/tb_renderer.cpp
	tb/tb_scroll_container.cpp
	tb/tb_scroller.cpp
	tb/tb_select.cpp
	tb/tb_select_item.cpp
	tb/tb_skin.cpp
	tb/tb_skin_util.cpp
	tb/tb_str.cpp
	tb/tb_style_edit.cpp
	tb/tb_style_edit_content.cpp
	tb/tb_system_sdl.cpp
	tb/tb_tab_container.cpp
	tb/tb_tempbuffer.cpp
	tb/tb_toggle_container.cpp
	tb/tb_value.cpp
	tb/tb_widget_skin_condition_context.cpp
	tb/tb_widget_value.cpp
	tb/tb_widgets.cpp
	tb/tb_widgets_common.cpp
	tb/tb_widgets_listener.cpp
	tb/tb_widgets_reader.cpp
	tb/tb_window.cpp
	tb/tb_bitmap_fragment.h
	tb/tb_color.h
	tb/tb_config.h
	tb/tb_core.h
	tb/tb_debug.h
	tb/tb_dimension.h
	tb/tb_editfield.h
	tb/tb_font_desc.h
	tb/tb_font_renderer.h
	tb/tb_geometry.h
	tb/tb_hash.h
	tb/tb_hashtable.h
	tb/tb_id.h
	tb/tb_inline_select.h
	tb/tb_language.h
	tb/tb_layout.h
	tb/tb_linklist.h
	tb/tb_list.h
	tb/tb_menu_window.h
	tb/tb_message_window.h
	tb/tb_msg.h
	tb/tb_node_ref_tree.h
	tb/tb_node_tree.h
	tb/tb_object.h
	tb/tb_popup_window.h
	tb/tb_renderer.h
	tb/tb_scroll_container.h
	tb/tb_scroller.h
	tb/tb_select.h
	tb/tb_select_item.h
	tb/tb_skin.h
	tb/tb_skin_util.h
	tb/tb_sort.h
	tb/tb_str.h
	tb/tb_style_edit.h
	tb/tb_style_edit_content.h
	tb/tb_system.h
	tb/tb_tab_container.h
	tb/tb_tempbuffer.h
	tb/tb_toggle_container.h
	tb/tb_types.h
	tb/tb_value.h
	tb/tb_widget_skin_condition_context.h
	tb/tb_widget_value.h
	tb/tb_widgets.h
	tb/tb_widgets_common.h
	tb/tb_widgets_listener.h
	tb/tb_widgets_reader.h
	tb/tb_window.h
)
set(LIB libturbobadger)
engine_add_library(
	LIB ${LIB}
	SRCS ${SRCS}
)
target_include_directories(${LIB} PUBLIC ../../../src/modules/ui/turbobadger tb)
target_link_libraries(${LIB} sdl2)

check_c_compiler_flag("-Wimplicit-fallthrough=0" HAS_IMPLICIT_FALLTHROUGH)
if (HAS_IMPLICIT_FALLTHROUGH)
	set(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -Wimplicit-fallthrough=0")
endif()
check_c_compiler_flag("-Wno-unused-value" HAS_NO_UNUSED_VALUE)
if (HAS_NO_UNUSED_VALUE)
	set(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -Wno-unused-value")
endif()
check_c_compiler_flag("-Wno-sign-compare" HAS_NO_SIGN_COMPARE)
if (HAS_NO_SIGN_COMPARE)
	set(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -Wno-sign-compare")
endif()
check_c_compiler_flag("-Wno-unused-variable" HAS_NO_UNUSED_VARIABLE)
if (HAS_NO_UNUSED_VARIABLE)
	set(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -Wno-unused-variable")
endif()
check_cxx_compiler_flag("-Wno-inconsistent-missing-override" HAS_INCONSISTENT_MISSING_OVERRIDE)
if (HAS_INCONSISTENT_MISSING_OVERRIDE)
	set(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -Wno-inconsistent-missing-override")
endif()
check_cxx_compiler_flag("-Wno-non-virtual-dtor" HAS_NO_NON_VIRTUAL_DTOR)
if (HAS_NO_NON_VIRTUAL_DTOR)
	set(ADDITIONAL_COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS} -Wno-non-virtual-dtor")
endif()
set_target_properties(${LIB} PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILE_FLAGS}")

if (WINDOWS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
	add_definitions(-DTB_USE_CURRENT_DIRECTORY)
elseif (LINUX)
elseif (DARWIN)
	add_definitions(-DMACOSX)
	add_definitions(-DTB_USE_CURRENT_DIRECTORY)
endif()
