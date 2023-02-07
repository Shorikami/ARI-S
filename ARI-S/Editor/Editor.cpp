#include <arpch.h>

#include "Editor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"

#include "Application.h"

#include "SceneSerializer.h"

#include "FileDialogs.h"

namespace ARIS
{
	Editor::Editor()
		: Layer("Editor")
		, m_ActiveScene(nullptr)
		, m_Framebuffer(nullptr)
	{
	}

	Editor::~Editor()
	{
		delete m_Framebuffer;
	}

	void Editor::OnAttach()
	{
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable keyboard controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;     // Enable docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable multi-viewport
		io.IniFilename = "Content/editor.ini";

		ImGui::StyleColorsDark();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetWindow());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 430");

		CustomizeColors();

		int w = app.GetWindow().GetWidth();
		int h = app.GetWindow().GetHeight();

		m_ActiveScene = std::make_shared<Scene>(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
		m_HierarchyPanel.SetContext(m_ActiveScene);
	}

	void Editor::OnDetach()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void Editor::OnUpdate(DeltaTime dt)
	{
		m_ActiveScene->OnViewportResize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));

		Framebuffer* fbo = m_ActiveScene.get()->GetSceneFBO();

		if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && 
			(fbo->GetSpecs().s_Width != m_ViewportSize.x || fbo->GetSpecs().s_Height != m_ViewportSize.y))
		{
			fbo->Resize(static_cast<uint32_t>(m_ViewportSize.x), static_cast<uint32_t>(m_ViewportSize.y));
		}

		BlockEvents(!m_ViewportFocused || !m_ViewportHovered);
		if (m_ViewportFocused)
		{
			m_ActiveScene->GetCamera().Update(dt);
		}

		m_ActiveScene.get()->Update(dt);
	}

	void Editor::Begin()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static bool dockspaceOpen = true;
		static bool optFullscreenPersistant = true;
		bool optFullScreen = optFullscreenPersistant;
		static ImGuiDockNodeFlags dockspaceFlags = ImGuiDockNodeFlags_None;

		ImGuiWindowFlags winFlags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (optFullScreen)
		{
			ImGuiViewport* vp = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(vp->Pos);
			ImGui::SetNextWindowSize(vp->Size);
			ImGui::SetNextWindowViewport(vp->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

			winFlags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			winFlags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}

		if (dockspaceFlags & ImGuiDockNodeFlags_PassthruCentralNode)
		{
			winFlags |= ImGuiWindowFlags_NoBackground;
		}

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Dockspace", &dockspaceOpen, winFlags);
		ImGui::PopStyleVar();

		if (optFullScreen)
		{
			ImGui::PopStyleVar(2);
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspaceID = ImGui::GetID("HysDockspace");
			ImGui::DockSpace(dockspaceID, ImVec2(0.0f, 0.0f), dockspaceFlags);
		}
	}

	void Editor::End()
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		io.DisplaySize = ImVec2(static_cast<float>(app.GetWindow().GetWidth()), static_cast<float>(app.GetWindow().GetHeight()));

		// Rendering
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup);
		}
	}

	void Editor::OnImGuiRender()
	{
		m_ActiveScene.get()->OnImGuiRender();

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New","Ctrl+N"))
				{
					NewScene();
				}

				if (ImGui::MenuItem("Open...", "Ctrl+O"))
				{
					OpenScene();
				}

				if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
				{
					SaveSceneAs();
				}

				if (ImGui::MenuItem("Exit"))
				{
					Application::Get().Close();
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		ImGui::End();

		m_HierarchyPanel.OnImGuiRender();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
		ImGui::Begin("Viewport");

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		ImVec2 vpPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { vpPanelSize.x, vpPanelSize.y };
		
		uint32_t fbTex = m_ActiveScene->GetSceneFBO()->GetColorAttachment().m_ID;
		ImGui::Image(reinterpret_cast<void*>(fbTex), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void Editor::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.m_Handled |= e.IsInCategory(CategoryMouse) & io.WantCaptureMouse;
			e.m_Handled |= e.IsInCategory(CategoryKeyboard) & io.WantCaptureKeyboard;
		}
		else
		{
			if (m_ViewportFocused)
			{
				m_ActiveScene->GetCamera().OnEvent(e);
			}

			m_ActiveScene.get()->OnEvent(e);
		}
	}

	void Editor::NewScene()
	{
		m_ActiveScene = std::make_shared<Scene>();
		//m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_HierarchyPanel.SetContext(m_ActiveScene);
	}

	void Editor::OpenScene()
	{
		std::string path = FileDialogs::OpenFile("Aris Scene (*.aris)\0*.aris\0");
		if (!path.empty())
		{
			std::shared_ptr<Scene> newScene = std::make_shared<Scene>();
			//m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			
			SceneSerializer s(newScene);
			if (s.Deserialize(path))
			{
				m_ActiveScene = newScene;
				m_HierarchyPanel.SetContext(m_ActiveScene);
			}
		}
	}

	void Editor::SaveSceneAs()
	{
		std::string path = FileDialogs::SaveFile("Aris Scene (*.aris)\0*.aris\0");
		if (!path.empty())
		{
			SceneSerializer s(m_ActiveScene);
			s.Serialize(path);
		}
	}

	void Editor::CustomizeColors()
	{
		ImGui::GetStyle().WindowRounding = 0.0f;

		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Text] = ImVec4{ 0.227f, 0.667f, 0.98f, 1.0f };
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.078f, 0.074f, 0.087f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.22f, 0.244f, 0.26f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.189f, 0.184f, 0.212f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		colors[ImGuiCol_SliderGrab] = ImVec4{ 0.392f, 0.361f, 0.498f, 1.0f };
		colors[ImGuiCol_SliderGrabActive] = ImVec4{ 0.492f, 0.461f, 0.598f, 1.0f };
	}
}