#include "raylib.h"
#include "loaddll.h"

constexpr Color MakeColor(double r, double g, double b, double a)
{
#define CLAMP01(v) (v < 0 ? 0 : (v > 1 ? 1 : v))
	return Color{ static_cast<unsigned char>(255 * CLAMP01(r)), static_cast<unsigned char>(255 * CLAMP01(g)), static_cast<unsigned char>(255 * CLAMP01(b)), static_cast<unsigned char>(255 * CLAMP01(a)) };
#undef CLAMP01
}

extern "C" __declspec(dllexport) double ClearBackgroundD(double r, double g, double b, double a)
{
	ClearBackground(MakeColor(r, g, b, a));
	return 0;
}

extern "C" __declspec(dllexport) double DrawPixelD(double x, double y, double r, double g, double b, double a)
{
	DrawPixel(static_cast<int>(x), static_cast<int>(y), MakeColor(r, g, b, a));
	return 0;
}

extern "C" __declspec(dllexport) double DrawRectangleD(double x, double y, double w, double h, double r, double g, double b, double a)
{
	DrawRectangle(static_cast<int>(x), static_cast<int>(y), static_cast<int>(w), static_cast<int>(h), MakeColor(r, g, b, a));
	return 0;
}

static InitFn Init;
static MainLoopFn MainLoop;

int main(int argc, char** argv)
{
	if (argc == 2)
	{
		if (!LoadDLL(Init, MainLoop, argv[1])) { return -1; }
	}
	else
	{
		if (!LoadDLL(Init, MainLoop)) { return -1; }
	}

	constexpr int w = 256, h = 256;

	InitWindow(w, h, "lang");
	SetTargetFPS(165);

	unsigned long long tick = 0;

	int mx = GetMouseX();
	int my = GetMouseY();

	RenderTexture2D rtex = LoadRenderTexture(w, h);

	BeginTextureMode(rtex);

	Init(w, h);

	EndTextureMode();

	while (!WindowShouldClose())
	{
		int tx = GetMouseX();
		int ty = GetMouseY();
		int dx = tx - mx;
		int dy = ty - my;
		mx = tx;
		my = ty;

		BeginTextureMode(rtex);

		MainLoop(w, h, tick++, dx, dy, GetMouseWheelMove(), IsMouseButtonDown(MOUSE_LEFT_BUTTON));

		EndTextureMode();

		BeginDrawing();

		ClearBackground(BLACK);
		DrawTexturePro(rtex.texture, Rectangle{ 0, 0, w, -h }, Rectangle{ 0, 0, w, h }, Vector2{ 0, 0 }, 0, WHITE);

		EndDrawing();
	}

	UnloadRenderTexture(rtex);

	CloseWindow();

	UnloadDLL();

	return 0;
}