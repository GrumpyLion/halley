#pragma once

#include "prec.h"
#include "ui/toolbar.h"

namespace Halley {
	class HalleyEditor;
	class EditorUIFactory;
	class Project;

	class EditorRootStage final : public Stage
	{
	public:
		EditorRootStage(HalleyEditor& editor, std::unique_ptr<Project> project, std::optional<String> launcherPath);
		~EditorRootStage();

		void init() override;
		void onVariableUpdate(Time time) override;
		void onRender(RenderContext& context) const override;

		void setSoftCursor(bool enabled);
		bool isSoftCursor() const;

		bool onQuitRequested() override;

	private:
		HalleyEditor& editor;
		I18N i18n;
		Executor mainThreadExecutor;

		std::unique_ptr<Project> project;
		std::optional<String> launcherPath;

		Sprite halleyLogo;
		Sprite cursor;

		Sprite background;
		Particles backgroundParticles;

		std::unique_ptr<EditorUIFactory> uiFactory;
		std::unique_ptr<UIRoot> ui;
		std::shared_ptr<UIWidget> topLevelUI;
		std::shared_ptr<ProjectWindow> projectWindow;

		std::unique_ptr<DevConServer> devConServer;

		bool softCursor = false;

		void initSprites();
		void createUI();

		void updateUI(Time time);
		void createLoadProjectUI();
		void setTopLevelUI(std::shared_ptr<UIWidget> ui);

		void loadProject();
		void unloadProject();
	};
}
