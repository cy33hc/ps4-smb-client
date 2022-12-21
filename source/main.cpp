#undef main

#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include <orbis/UserService.h>
#include <orbis/SystemService.h>
#include <orbis/Pad.h>
#include <orbis/AudioOut.h>
#include <orbis/Net.h>
#include <dbglogger.h>

#include "imgui.h"
#include "SDL2/SDL.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "config.h"
#include "lang.h"
#include "gui.h"
#include "util.h"
extern "C"
{
#include "orbis_jbc.h"
}

#define FRAME_WIDTH 1920
#define FRAME_HEIGHT 1080
#define NET_HEAP_SIZE   (5 * 1024 * 1024)

// SDL window and software renderer
SDL_Window *window;
SDL_Renderer *renderer;

void InitImgui()
{
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	(void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	io.Fonts->Clear();
	io.Fonts->Flags |= ImFontAtlasFlags_NoBakedLines;

	static const ImWchar ranges[] = { // All languages with chinese included
		0x0020, 0x00FF, // Basic Latin + Latin Supplement
		0x0100, 0x024F, // Latin Extended
		0x0370, 0x03FF, // Greek
		0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
		0x0590, 0x05FF, // Hebrew
		0x1E00, 0x1EFF, // Latin Extended Additional
		0x1F00, 0x1FFF, // Greek Extended
		0x2000, 0x206F, // General Punctuation
		0x2100, 0x214F, // Letterlike Symbols
		0x2460, 0x24FF, // Enclosed Alphanumerics
		0x2DE0, 0x2DFF, // Cyrillic Extended-A
		0x2E80, 0x2EFF, // CJK Radicals Supplement
		0x3000, 0x30FF, // CJK Symbols and Punctuations, Hiragana, Katakana
		0x31F0, 0x31FF, // Katakana Phonetic Extensions
		0x3400, 0x4DBF, // CJK Rare
		0x4E00, 0x9FFF, // CJK Ideograms
		0xA640, 0xA69F, // Cyrillic Extended-B
		0xF900, 0xFAFF, // CJK Compatibility Ideographs
		0xFF00, 0xFFEF, // Half-width characters
		0,
	};

	std::string lang = std::string(language);
	int32_t lang_idx;
	sceSystemServiceParamGetInt( ORBIS_SYSTEM_SERVICE_PARAM_ID_LANG, &lang_idx );

	lang = Util::Trim(lang, " ");
	if (lang.compare("Korean") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_KOREAN))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 26.0f, NULL, io.Fonts->GetGlyphRangesKorean());
	}
	else if (lang.compare("Simplified Chinese") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_CHINESE_S))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 26.0f, NULL, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
	}
	else if (lang.compare("Traditional Chinese") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_CHINESE_T))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
	}
	else if (lang.compare("Japanese") == 0 || lang.compare("Ryukyuan") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_JAPANESE))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 26.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	}
	else if (lang.compare("Thai") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_THAI))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 26.0f, NULL, io.Fonts->GetGlyphRangesThai());
	}
	else if (lang.compare("Vietnamese") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_VIETNAMESE))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 26.0f, NULL, io.Fonts->GetGlyphRangesVietnamese());
	}
	else if (lang.compare("Greek") == 0 || (lang.empty() && lang_idx == ORBIS_SYSTEM_PARAM_LANG_GREEK))
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto_ext.ttf", 26.0f, NULL, io.Fonts->GetGlyphRangesGreek());
	}
	else
	{
		io.Fonts->AddFontFromFileTTF("/app0/assets/fonts/Roboto.ttf", 26.0f, NULL, ranges);
	}
	Lang::SetTranslation(lang_idx);

	auto &style = ImGui::GetStyle();
	style.AntiAliasedLinesUseTex = false;
	style.AntiAliasedLines = true;
	style.AntiAliasedFill = true;
	style.WindowRounding = 1.0f;
	style.FrameRounding = 2.0f;
	style.GrabRounding = 2.0f;

	ImVec4 *colors = ImGui::GetStyle().Colors;
	colors[ImGuiCol_Text] = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
	colors[ImGuiCol_Border] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.20f, 0.28f, 1.00f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.09f, 0.12f, 0.14f, 1.00f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.09f, 0.12f, 0.14f, 0.65f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.08f, 0.10f, 0.12f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.39f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.18f, 0.22f, 0.25f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.09f, 0.21f, 0.31f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.28f, 0.56f, 1.00f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.37f, 0.61f, 1.00f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_Header] = ImVec4(0.20f, 0.25f, 0.29f, 0.55f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_Separator] = ImVec4(0.20f, 0.25f, 0.29f, 1.00f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.10f, 0.40f, 0.75f, 0.78f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.10f, 0.40f, 0.75f, 1.00f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_Tab] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_TabActive] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.11f, 0.15f, 0.17f, 1.00f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.00f, 0.50f, 0.50f, 1.0f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
}

static void terminate()
{
	terminate_jbc();
	sceSystemServiceLoadExec("exit", NULL);
}

int main()
{
	dbglogger_init();
	dbglogger_log("If you see this you've set up dbglogger correctly.");
	int rc;
	// No buffering
	setvbuf(stdout, NULL, _IONBF, 0);

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
	{
		return 0;
	}

	// load common modules
	int ret = sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_SYSTEM_SERVICE);
	if (ret < 0)
	{
		return 0;
	}

	ret = sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_USER_SERVICE);
	if (ret < 0)
	{
		return 0;
	}

	if (sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_PAD) < 0)
		return 0;

	if (sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_AUDIOOUT) < 0 ||
		sceAudioOutInit() != 0)
	{
		return 0;
	}

	if (sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_IME_DIALOG) < 0)
		return 0;

	if(sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_NET) < 0 || sceNetInit() != 0)
		return 0;

    sceNetPoolCreate("simple", NET_HEAP_SIZE, 0);

	CONFIG::LoadConfig();

	// Create a window context
	window = SDL_CreateWindow("main", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, FRAME_WIDTH, FRAME_HEIGHT, 0);
	if (window == NULL)
		return 0;

	renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL)
		return 0;

	InitImgui();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
	ImGui_ImplSDLRenderer_Init(renderer);
	ImGui_ImplSDLRenderer_CreateFontsTexture();
	ImGui_ImplSDL2_DisableButton(SDL_CONTROLLER_BUTTON_X, true);

	if (!initialize_jbc())
	{
		terminate();
	}
	atexit(terminate);

	GUI::RenderLoop(renderer);

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	ImGui::DestroyContext();

	return 0;
}