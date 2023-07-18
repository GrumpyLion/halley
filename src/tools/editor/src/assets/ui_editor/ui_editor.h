#pragma once

#include <halley.hpp>
#include "ui_widget_editor.h"
#include "ui_widget_list.h"
#include "../asset_editor.h"
#include "halley/tools/dll/project_dll.h"
#include "src/scene/choose_window.h"

namespace Halley {
	class UIEditorDisplay;
	class UIGraphNode;

	class UIEditor : public AssetEditor, IProjectDLLListener {
	public:
		UIEditor(UIFactory& factory, Resources& gameResources, Project& project, ProjectWindow& projectWindow, const HalleyAPI& api);
		~UIEditor() override;

		void update(Time t, bool moved) override;

		void reload() override;
		void onMakeUI() override;
		void markModified();
		void onWidgetModified(const String& id);
		void reselectWidget();

		bool isModified() override;
		void save() override;

		UIFactory& getGameFactory();

		bool onKeyPress(KeyboardKeyPress key) override;

        void copySelection();
        void pasteSelection();
        void cutSelection();
        void deleteSelection();

	protected:
		void onProjectDLLStatusChange(ProjectDLL::Status status) override;
		
	private:
		std::shared_ptr<const Resource> loadResource(const String& assetId) override;
		
		std::unique_ptr<UIFactory> gameFactory;
		std::unique_ptr<I18N> gameI18N;
		std::shared_ptr<UIWidgetList> widgetList;
		std::shared_ptr<UIWidgetEditor> widgetEditor;
		ProjectWindow& projectWindow;

		std::shared_ptr<UIDefinition> uiDefinition;
		std::shared_ptr<UIEditorDisplay> display;
		bool loaded = false;
		bool modified = false;
		bool pendingLoad = false;

		String curSelection;

		void open();
		void doLoadUI();
		void setSelectedWidget(const String& id);

		void addWidget();
		void addWidget(const String& widgetClass);
		void addWidget(const String& referenceId, bool asChild, ConfigNode data);
		void removeWidget();
		void removeWidget(const String& id);

		void loadGameFactory();
	};

	class ChooseUIWidgetWindow : public ChooseAssetWindow {
	public:
		ChooseUIWidgetWindow(UIFactory& factory, UIFactory& gameFactory, Callback callback);

	protected:
		std::shared_ptr<UIImage> makeIcon(const String& id, bool hasSearch) override;
		LocalisedString getItemLabel(const String& id, const String& name, bool hasSearch) override;
		void sortItems(Vector<std::pair<String, String>>& items) override;

		int getNumColumns(Vector2f scrollPaneSize) const override;

	private:
		UIFactory& factory;
		UIFactory& gameFactory;
	};
}
