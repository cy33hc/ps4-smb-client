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
//#include <dbglogger.h>

#include "imgui.h"
#include "SDL2/SDL.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdlrenderer.h"
#include "config.h"
#include "lang.h"
#include "gui.h"
#include "util.h"
#include "installer.h"

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

	style.Colors[ImGuiCol_Text] = ImVec4( 236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_TextDisabled] = ImVec4( 236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 0.58f );
	style.Colors[ImGuiCol_WindowBg] = ImVec4( 44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 0.95f );
	style.Colors[ImGuiCol_ChildBg] = ImVec4( 57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.58f );
	style.Colors[ImGuiCol_Border] = ImVec4( 44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 0.00f );
	style.Colors[ImGuiCol_BorderShadow] = ImVec4( 44.f / 255.f, 62.f / 255.f, 80.f / 255.f, 0.00f );
	style.Colors[ImGuiCol_FrameBg] = ImVec4( 57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.78f );
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_TitleBg] = ImVec4( 57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4( 57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.75f );
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4( 57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 0.47f );
	style.Colors[ImGuiCol_ScrollbarBg] = ImVec4( 57.f / 255.f, 79.f / 255.f, 105.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.21f );
	style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.78f );
	style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_CheckMark] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.80f );
	style.Colors[ImGuiCol_SliderGrab] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.50f );
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_Button] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.50f );
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.86f );
	style.Colors[ImGuiCol_ButtonActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_Header] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.76f );
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.86f );
	style.Colors[ImGuiCol_HeaderActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_ResizeGrip] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.15f );
	style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.78f );
	style.Colors[ImGuiCol_ResizeGripActive] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_PlotLines] = ImVec4( 236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 0.63f );
	style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_PlotHistogram] = ImVec4( 236.f / 255.f, 240.f / 255.f, 241.f / 255.f, 0.63f );
	style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 1.00f );
	style.Colors[ImGuiCol_TextSelectedBg] = ImVec4( 41.f / 255.f, 128.f / 255.f, 185.f / 255.f, 0.43f );
	style.Colors[ImGuiCol_PopupBg] = ImVec4( 33.f / 255.f, 46.f / 255.f, 60.f / 255.f, 0.92f );
}

static void terminate()
{
	INSTALLER::Exit();
	terminate_jbc();
	sceSystemServiceLoadExec("exit", NULL);
}

int main()
{
	//dbglogger_init();
	//dbglogger_log("If you see this you've set up dbglogger correctly.");
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

	ret = sceSysmoduleLoadModuleInternal(ORBIS_SYSMODULE_INTERNAL_BGFT);
	if (ret) {
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

	if (INSTALLER::Init() < 0)
		return 0;

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