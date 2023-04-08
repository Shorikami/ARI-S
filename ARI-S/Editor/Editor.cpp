#include <arpch.h>

#include "Editor.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_opengl3_loader.h"

#include "Application.h"

#include "SceneSerializer.h"

#include "FileDialogs.h"

#include "ImGuizmo.h"

#include "InputPoll.h"
#include "Tools.h"

#include "Math/Math.h"

#include "../Rendering/DebugDraw.h"

namespace ARIS
{
	// TODO: Change later
	extern const std::filesystem::path s_AssetPath;

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

		float fontSize = 14.0f;
		io.FontDefault = io.Fonts->AddFontFromFileTTF("Resources/Fonts/Inconsolata/Inconsolata-Regular.ttf", fontSize);

		ImGui::StyleColorsDark();

		Application& app = Application::Get();
		GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetWindow());

		ImGui_ImplGlfw_InitForOpenGL(window, true);
		ImGui_ImplOpenGL3_Init("#version 450");

		CustomizeColors();

		int w = app.GetWindow().GetWidth();
		int h = app.GetWindow().GetHeight();

		m_ActiveScene = std::make_shared<Scene>(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());
		m_HierarchyPanel.SetContext(m_ActiveScene);

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 1.0f, 100.0f);

		//m_Framebuffer = new Framebuffer(w, h, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//m_Framebuffer->Bind();
		//m_Framebuffer->AllocateAttachTexture(GL_COLOR_ATTACHMENT0, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);
		//m_Framebuffer->AllocateAttachTexture(GL_COLOR_ATTACHMENT1, GL_R32I, GL_RED_INTEGER, GL_UNSIGNED_BYTE);
		//m_Framebuffer->DrawBuffers();
		//m_Framebuffer->AllocateAttachTexture(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		//
		//m_Framebuffer->Unbind();
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

		if (FramebufferSpecs spec = m_ActiveScene->GetSceneFBO()->GetSpecs(); m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f
			&& (spec.s_Width != m_ViewportSize.x || spec.s_Height != m_ViewportSize.y))
		{
			m_EditorCamera.SetViewportSize(static_cast<int>(m_ViewportSize.x), static_cast<int>(m_ViewportSize.y));
		}

		//m_Framebuffer->Bind();
		//m_Framebuffer->ClearAttachment(1, -1);
		//m_Framebuffer->Unbind();

		m_EditorCamera.OnUpdate(dt);
		DebugWrapper::GetInstance().Update(m_EditorCamera);

		m_ActiveScene->UpdateEditor(dt, m_EditorCamera);


		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_VPBounds[0].x;
		my -= m_VPBounds[0].y;
		
		glm::vec2 vpSize = m_VPBounds[1] - m_VPBounds[0];
		my = vpSize.y - my;
		
		int mouseX = static_cast<int>(mx);
		int mouseY = static_cast<int>(my);
		
		if (mouseX >= 0 && mouseY >= 0 &&
			mouseX < static_cast<int>(vpSize.x) && mouseY < static_cast<int>(vpSize.y))
		{
			int pixel = m_ActiveScene->GetSceneFBO()->ReadPixel(1, mouseX, mouseY);
			if (pixel == -1)
			{
				m_HoveredEntity = Entity();
			}
			else
			{
				m_HoveredEntity = Entity((entt::entity)pixel, m_ActiveScene.get());
			}
		}
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
		m_ContentBrowser.OnImGuiRender();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0,0 });
		ImGui::Begin("Viewport");
		auto vpMinRegion = ImGui::GetWindowContentRegionMin();
		auto vpMaxRegion = ImGui::GetWindowContentRegionMax();
		auto vpOffset = ImGui::GetWindowPos();
		m_VPBounds[0] = { vpMinRegion.x + vpOffset.x, vpMinRegion.y + vpOffset.y };
		m_VPBounds[1] = { vpMaxRegion.x + vpOffset.x, vpMaxRegion.y + vpOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();

		BlockEvents(!m_ViewportHovered);

		ImVec2 vpPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { vpPanelSize.x, vpPanelSize.y };
		
		uint32_t fbTex = m_ActiveScene->GetSceneFBO()->GetColorAttachment(0).m_ID;
		ImGui::Image((void*)(intptr_t)fbTex, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, 
			ImVec2{ 0, 1 }, ImVec2{ 1, 0 });
		
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;
				OpenScene(std::filesystem::path(s_AssetPath) / path);
			}
			

			ImGui::EndDragDropTarget();
		}


		Entity selectedEntity = m_HierarchyPanel.GetSelectedEntity();
		if (selectedEntity && m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_VPBounds[0].x, m_VPBounds[0].y,
				m_VPBounds[1].x - m_VPBounds[0].x, m_VPBounds[1].y - m_VPBounds[0].y);

			auto fbo = m_ActiveScene->GetSceneFBO();

			const glm::mat4& proj = m_EditorCamera.GetProjection();
			glm::mat4 view = m_EditorCamera.GetViewMatrix();

			auto& tc = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 tr = tc.GetTransform();

			bool snap = InputPoll::IsKeyPressed(KeyTags::LeftControl);
			float snapVal = 0.5f;

			if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
			{
				snapVal = 45.0f;
			}

			float snapVals[3] = { snapVal, snapVal, snapVal };

			ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(proj), (ImGuizmo::OPERATION)m_GizmoType,
				ImGuizmo::LOCAL, glm::value_ptr(tr), nullptr, snap ? snapVals : nullptr);

			if (ImGuizmo::IsUsing())
			{
				glm::vec3 tra, rot, sc;
				Math::Decompose(tr, tra, rot, sc);

				glm::vec3 deltaRot = rot - tc.m_Rotation;
				tc.m_Translation = tra;
				tc.m_Rotation += deltaRot;
				tc.m_Scale = sc;
			}

		}

		ImGui::End();
		ImGui::PopStyleVar();

		//ImGui::End();
	}

	void Editor::OnEvent(Event& e)
	{
		if (m_BlockEvents)
		{
			ImGuiIO& io = ImGui::GetIO();
			e.m_Handled |= e.IsInCategory(CategoryMouse) & io.WantCaptureMouse;
			e.m_Handled |= e.IsInCategory(CategoryKeyboard) & io.WantCaptureKeyboard;
		}

		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FUNC(Editor::OnKeyPressed));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FUNC(Editor::OnMouseButtonPressed));
	}

	bool Editor::OnKeyPressed(KeyPressedEvent& e)
	{
		if (e.IsRepeat())
		{
			return false;
		}

		bool ctrl = InputPoll::IsKeyPressed(KeyTags::LeftControl) || InputPoll::IsKeyPressed(KeyTags::RightControl);
		bool shift = InputPoll::IsKeyPressed(KeyTags::LeftShift) || InputPoll::IsKeyPressed(KeyTags::RightShift);

		switch (e.GetKeyCode())
		{
			case KeyTags::N:
			{
				if (ctrl)
				{
					NewScene();
				}
				break;
			}

			case KeyTags::O:
			{
				if (ctrl)
				{
					OpenScene();
				}
				break;
			}

			case KeyTags::S:
			{
				if (ctrl)
				{
					if (shift)
					{
						SaveSceneAs();
					}
				}
				break;
			}

			case KeyTags::Q:
			{
				if (!ImGuizmo::IsUsing())
				{
					m_GizmoType = -1;
				}
				break;
			}

			case KeyTags::W:
			{
				if (!ImGuizmo::IsUsing())
				{
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				}
				break;
			}
			case KeyTags::E:
			{
				if (!ImGuizmo::IsUsing())
				{
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				}
				break;
			}
			case KeyTags::R:
			{
				if (!ImGuizmo::IsUsing())
				{
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				}
				break;
			}

			//case KeyTags::Delete:
			//{
			//	if (Application::Get()->GetActiveWidgetID() == 0)
			//	{
			//		Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
			//		if (selectedEntity)
			//		{
			//			m_SceneHierarchyPanel.SetSelectedEntity({});
			//			m_ActiveScene->DestroyEntity(selectedEntity);
			//		}
			//	}
			//	break;
			//}
		}

		return false;
	}

	bool Editor::OnMouseButtonPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == MouseTags::ButtonLeft)
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver() && !InputPoll::IsKeyPressed(KeyTags::LeftAlt))
			{
				m_HierarchyPanel.SetSelectedEntity(m_HoveredEntity);
			}
		}

		return false;
	}

	void Editor::NewScene()
	{
		m_ActiveScene = std::make_shared<Scene>(Application::Get().GetWindow().GetWidth(),
			Application::Get().GetWindow().GetHeight());
		//m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_HierarchyPanel.SetContext(m_ActiveScene);
	}

	void Editor::OpenScene()
	{
		std::string path = FileDialogs::OpenFile("Aris Scene (*.aris)\0*.aris\0");
		if (!path.empty())
		{
			OpenScene(path);
		}
	}

	void Editor::OpenScene(const std::filesystem::path& path)
	{
		m_ActiveScene = std::make_shared<Scene>(Application::Get().GetWindow().GetWidth(),
			Application::Get().GetWindow().GetHeight());
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		m_HierarchyPanel.SetContext(m_ActiveScene);

		SceneSerializer s(m_ActiveScene);
		s.Deserialize(path.string());
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
		ImGui::GetStyle().FramePadding = ImVec2(4.0f, 4.0f);
		ImGui::GetStyle().ItemSpacing = ImVec2(8.0f, 10.0f);
		ImGui::GetStyle().ScrollbarSize = 10.0f;
		ImGui::GetStyle().FrameBorderSize = 1.0f;

		ImGui::GetStyle().GrabMinSize = 8.0f;
		ImGui::GetStyle().GrabRounding = 12.0f;

		ImGui::GetStyle().FrameRounding = 12.0f;
		ImGui::GetStyle().ScrollbarRounding = 12.0f;
		ImGui::GetStyle().TabRounding = 4.0f;

		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_Text] = ImVec4{ 0.627f, 0.667f, 0.698f, 1.0f };
		colors[ImGuiCol_WindowBg] = ImVec4{ 0.128f, 0.124f, 0.137f, 1.0f };

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