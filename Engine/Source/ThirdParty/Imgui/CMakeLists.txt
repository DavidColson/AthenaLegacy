project(Imgui)

set(SOURCES
    "imgui.cpp"
    "imgui_demo.cpp"
    "imgui_draw.cpp"
    "imgui_widgets.cpp"
    "imconfig.h"
    "imgui.h"
    "imgui_internal.h"
    "imstb_rectpack.h"
    "imstb_textedit.h"
    "imstb_truetype.h"
    "misc/cpp/imgui_stdlib.cpp"
    "misc/cpp/imgui_stdlib.h"
    "misc/freetype/imgui_freetype.cpp"
    "misc/freetype/imgui_freetype.h"
	"examples/imgui_impl_dx11.cpp"
	"examples/imgui_impl_dx11.h"
	"examples/imgui_impl_sdl.cpp"
	"examples/imgui_impl_sdl.h"
    )

include_directories("../../" "/")
add_library (Imgui ${SOURCES})