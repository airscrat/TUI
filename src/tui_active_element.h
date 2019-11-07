#pragma once

namespace tui
{
	struct active_element
	{
	private:
		bool active = false;
	protected:
		virtual void activationAction() {}
		virtual void disactivationAction() {}
	public:
		void activate()
		{
			active = true;
			activationAction();
		}
		void disactivate()
		{
			active = false;
			disactivationAction();
		}
		bool isActive() { return active; }
	};
}