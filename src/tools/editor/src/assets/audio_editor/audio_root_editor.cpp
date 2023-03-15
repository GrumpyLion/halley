#include "audio_root_editor.h"

#include "audio_object_editor.h"
#include "halley/audio/audio_object.h"
#include "halley/properties/audio_properties.h"
using namespace Halley;

AudioRootEditor::AudioRootEditor(UIFactory& factory, AudioObjectEditor& editor, AudioObject& object)
	: UIWidget("audio_root_editor", Vector2f(), UISizer())
	, factory(factory)
	, editor(editor)
	, object(object)
{
	factory.loadUI(*this, "halley/audio_editor/audio_root_editor");
}

void AudioRootEditor::onMakeUI()
{
	getWidgetAs<UIDropdown>("bus")->setOptions(editor.getAudioProperties().getBusIds());

	bindData("bus", object.getBus(), [=] (String value)
	{
		object.setBus(std::move(value));
		editor.markModified(false);
	});

	bindData("gainMin", object.getGain().start, [=] (float value)
	{
		object.getGain().start = value;
		editor.markModified(false);
	});

	bindData("gainMax", object.getGain().end, [=] (float value)
	{
		object.getGain().end = value;
		editor.markModified(false);
	});

	bindData("pitchMin", object.getPitch().start, [=] (float value)
	{
		object.getPitch().start = value;
		editor.markModified(false);
	});

	bindData("pitchMax", object.getPitch().end, [=] (float value)
	{
		object.getPitch().end = value;
		editor.markModified(false);
	});
}
