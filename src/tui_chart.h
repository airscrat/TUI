#pragma once
#include "tui_console.h"
#include "tui_appearance.h"
#include "tui_text_utils.h"
#include "tui_scroll.h"
#include "tui_bar.h"
#include <vector>

namespace tui
{
	struct chart_appearance : appearance
	{
	protected:
		symbol full;
		symbol lower_half;
		symbol upper_half;
	public:
		chart_appearance() : chart_appearance(U'\x2588', U'\x2584', U'\x2580') {}
		chart_appearance(symbol Full, symbol Lower, symbol Upper) : full(Full), lower_half(Lower), upper_half(Upper) {}

		void setColor(color Color) override
		{
			full.setColor(Color);
			setAppearance_action();
		}
		void setAppearance(chart_appearance appearance) 
		{
			*this = appearance; 
			setAppearance_action();
		}
		chart_appearance getAppearance() { return *this; }
		void setFullSymbol(symbol Full) 
		{
			full = Full;
			setAppearance_action();
		}
		symbol getFullSymbol() { return full; }
		void setLowerHalfSymbol(symbol Lower)
		{
			lower_half = Lower;
			setAppearance_action();
		}
		symbol getLowerHalfSymbol() { return lower_half; }
		void setUpperHalfSymbol(symbol Upper)
		{
			upper_half = Upper;
			setAppearance_action();
		}
		symbol getUpperHalfSymbol() { return upper_half; }
	};



	struct chart : surface, chart_appearance, active_element
	{
	private:
		std::vector<float> m_values;
		tui::scroll<tui::DIRECTION::HORIZONTAL> m_scroll;
		unsigned int m_distance = 2;

		bool m_redraw_needed = true;

		void fill()
		{
			surface::makeTransparent();

			auto getMin = [&]()
			{
				if (m_values.size() == 0) { return (float)0; }
				else
				{
					float min = m_values[0];
					{
						for (int i = 0; i < m_values.size(); i++)
						{
							if (m_values[i] < min) { min = m_values[i]; }
						}
					}
					return min;
				}
			};
			auto getMax = [&]()
			{
				if (m_values.size() == 0) { return (float)0; }
				else
				{
					float max = m_values[0];
					{
						for (int i = 0; i < m_values.size(); i++)
						{
							if (m_values[i] > max) { max = m_values[i]; }
						}
					}
					return max;
				}
			};
			float min = getMin();
			float max = getMax();

			m_scroll.setContentLength(m_values.size() * m_distance - (m_distance-1));

			auto getHeight = [&]()
			{
				if (m_scroll.isNeeded()) { return getSize().y - 1; }
				else { return getSize().y; }
			};

			int distance = ceil(fabs(min - max));
			if (min > 0) { distance += min; }
			if (max < 0) { distance += fabs(max); }

			if (distance > 0)
			{
				int halves = getSize().y * 2;
				int p_halves = round(max / (float)distance * halves) * (max>=0);

				int h_pos = m_scroll.getHandlePosition();
				int x = m_distance *(h_pos % m_distance != 0) - h_pos % m_distance;

				for (int i = ceil(h_pos / (float)m_distance); (i < m_values.size() && x < getSize().x); i++, x += m_distance)
				{
					int h = round(fabs(m_values[i]) / (float)distance * halves);

					auto isFull = [&](int y)
					{
						switch (m_values[i] >= 0)
						{
						case true:
							if (y >= p_halves - h && y < p_halves) { return true; }
							break;
						case false:
							if (y >= p_halves && y < p_halves + h) { return true; }
						}
						return false;
					};
						
					for (int j = 0; j < halves; j+=2)
					{
						if (isFull(j) && isFull(j + 1))
						{
							surface::setSymbolAt(full, { x, j / 2 });
						}
						else if (isFull(j) && !isFull(j + 1))
						{
							surface::setSymbolAt(upper_half, { x, j / 2 });
						}
						else if (!isFull(j) && isFull(j + 1))
						{
							surface::setSymbolAt(lower_half, { x, j / 2 });
						}
					}	
				}
			}
			surface::insertSurface(m_scroll);
		}

	public:
		int key_left = tui::KEYBOARD::KEY::LEFT;
		int key_right = tui::KEYBOARD::KEY::RIGHT;
		chart() : m_scroll({0,100}) 
		{
			m_scroll.setPosition({ { 0,-1 }, { 0,100 } });
		}

		void setValues(std::vector<float> values) 
		{ 
			m_values = values;
			m_redraw_needed = true;
		}
		std::vector<float> getValues() { return m_values; }

		void setDistance(unsigned int distance)
		{
			if (distance > 0) { m_distance = distance; }
			else { m_distance = 1; }
			m_redraw_needed = true;
		}
		unsigned int getDistance() { return m_distance; }

		void update()
		{
			int old_scroll_handle_pos = m_scroll.getHandlePosition();
			m_scroll.immobilize(false);
			m_scroll.update();
			m_scroll.immobilize(true);
			if (m_scroll.getHandlePosition() != old_scroll_handle_pos)
			{
				m_redraw_needed = true;
			}

		}

		void resize_action() override { m_redraw_needed = true; }
		void draw_action() override 
		{
			update();
			if (m_redraw_needed)
			{
				fill();
				m_redraw_needed = false;
			}
		}

		void activation_action() override 
		{
			m_scroll.activate(); 
			if (m_scroll.isNeeded()) { m_redraw_needed = true; }
		}
		void disactivation_action() override 
		{
			m_scroll.disactivate(); 
			if (m_scroll.isNeeded()) { m_redraw_needed = true; }
		}

		void setAppearance_action() override { m_redraw_needed = true; }
	};


}