#pragma once

class GuiWindow {
public:
	virtual ~GuiWindow() = default;

	void virtual render() = 0;
};
