#include "halley/input/text_input_capture.h"
#include <memory>


#include "halley/input/input_keyboard.h"
#include "halley/input/text_input_data.h"

using namespace Halley;


void ITextInputCapture::onTextEntered(const StringUTF32& text)
{
}

bool ITextInputCapture::onKeyPress(KeyboardKeyPress c, IClipboard* clipboard)
{
	return false;
}

StandardTextInputCapture::StandardTextInputCapture(InputKeyboard& parent)
	: parent(parent)
{
}

StandardTextInputCapture::~StandardTextInputCapture()
{
	parent.removeCapture(this);
}

void StandardTextInputCapture::open(TextInputData& input, SoftwareKeyboardData softKeyboardData)
{
	currentlyOpen = true;
	textInput = &input;
}

void StandardTextInputCapture::close()
{
	currentlyOpen = false;
	textInput = nullptr;
}

bool StandardTextInputCapture::isOpen() const
{
	return currentlyOpen;
}

void StandardTextInputCapture::update()
{
}

void StandardTextInputCapture::onTextEntered(const StringUTF32& text)
{
	textInput->insertText(text);
}

bool StandardTextInputCapture::onKeyPress(KeyboardKeyPress c, IClipboard* clipboard)
{
	return textInput->onKeyPress(c, clipboard);
}

TextInputCapture::TextInputCapture(TextInputData& inputData, SoftwareKeyboardData softKeyboardData, std::unique_ptr<ITextInputCapture> _capture)
	: capture(std::move(_capture))
{
	capture->open(inputData, std::move(softKeyboardData));
}

TextInputCapture::~TextInputCapture()
{
	if (capture) {
		capture->close();
	}
}

bool TextInputCapture::update() const
{
	if (capture->isOpen()) {
		capture->update();
	}
	return capture->isOpen();
}
