#include "imgui.h"
#include "hud_imgui_dialogbox.h"

#include "fontawesome_brands.h"

DialogButtons CClientImguiDialogBox::DrawDialogBox(std::string title, std::string message, DialogButtons_ dialogButtons, ImVec2 size, bool forceSize)
{
	title = ICON_FA_STEAM_SYMBOL + std::string(" ") + title;

	if ((size.x != 0 && size.y != 0) || forceSize)
		ImGui::SetNextWindowSize(ImVec2(size));

	ImGui::OpenPopup(title.c_str());
	if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse))
	{
		ImGui::Text(message.c_str());
		ImGui::Separator();
		
		if (dialogButtons & DialogButtons::OK)
		{
			if (ImGui::Button("OK", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::OK;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Yes)
		{
			if (ImGui::Button("Yes", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Yes;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::No)
		{
			if (ImGui::Button("No", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::No;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Retry)
		{
			if (ImGui::Button("Retry", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Retry;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Restart)
		{
			if (ImGui::Button("Restart", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Restart;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Ignore)
		{
			if (ImGui::Button("Ignore", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Ignore;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Continue)
		{
			if (ImGui::Button("Continue", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Continue;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Quit)
		{
			if (ImGui::Button("Quit", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Quit;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Abort)
		{
			if (ImGui::Button("Abort", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Abort;
			}
			ImGui::SameLine();
		}

		if (dialogButtons & DialogButtons::Cancel)
		{
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
				return DialogButtons::Cancel;
			}
			ImGui::SameLine();
		}

		ImGui::EndPopup();
	}

	return DialogButtons::None;
}
