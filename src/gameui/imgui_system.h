//=============================================================================//
// 
// Purpose: Dear ImGui engine implementation
// 
//=============================================================================//
#ifndef IMGUI_SYSTEM_H
#define IMGUI_SYSTEM_H
#include "imgui/misc/imgui_snapshot.h"
#include "imgui_surface.h"

class CImguiSystem
{
public:
	CImguiSystem();

	bool Init();
	void Shutdown();

	void AddSurface(CImguiSurface* const surface);
	void RemoveSurface(CImguiSurface* const surface);

	void SwapBuffers();

	void SampleFrame();
	void RenderFrame();

	// statics:
	static LRESULT MessageHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// inlines:
	inline bool IsInitialized() const { return m_initialized; };

private:
	ImDrawDataSnapshot m_snapshotData;
	CUtlVector<CImguiSurface* const> m_surfaceList;

	// Mutex used during swapping and rendering, we draw the windows in the
	// main thread, and render it in the render thread. The only place this
	// mutex is used is during snapshot swapping and during rendering
	mutable CThreadMutex m_snapshotBufferMutex;

	// Mutex used between ImGui window procedure handling and drawing, see
	// https://github.com/ocornut/imgui/issues/6895. In this engine the window
	// is ran in thread separate from the main thread, therefore it needs a
	// lock to control access as main calls SampleFrame().
	mutable CThreadMutex m_inputEventQueueMutex;

	bool m_initialized;
	bool m_hasNewFrame;
};

CImguiSystem* ImguiSystem();

#endif // IMGUI_SYSTEM_H
