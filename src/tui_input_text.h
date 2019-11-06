#pragma once
#include "tui_surface.h"
#include "tui_appearance.h"
#include "tui_text_utils.h"

namespace tui
{
	struct input_text_appearance : appearance
	{
	protected:
		symbol m_insert_cursor;
		symbol m_overtype_cursor;

		text_appearance m_text_appearance;
	public:
		input_text_appearance() : input_text_appearance('_', U'\x2584', text_appearance()) {}
		input_text_appearance(symbol insert_cursor, symbol overtype_cursor, text_appearance txt_appearance) 
			: m_insert_cursor(insert_cursor), m_overtype_cursor(overtype_cursor), m_text_appearance(txt_appearance){}

		void setColor(color Color) override
		{
			m_insert_cursor.setColor(Color);
			m_overtype_cursor.setColor(Color);
			m_text_appearance.setColor(Color);
			setAppearanceAction();
		}

		void setAppearance(input_text_appearance appearance) { setElement(*this, appearance); }
		input_text_appearance getAppearance() { return *this; }

		void setInsertCursorSymbol(symbol Cursor) { setElement(m_insert_cursor,Cursor); }
		symbol getInsertCursorSymbol() { return m_insert_cursor; }

		void setOvertypeCursorSymbol(symbol Cursor) { setElement(m_overtype_cursor, Cursor); }
		symbol getOvertypeCursorSymbol() { return m_overtype_cursor; }

		void setTextAppearance(text_appearance appearance) { setElement(m_text_appearance, appearance); }
		text_appearance getTextAppearance() { return m_text_appearance; }
	};

	struct input_text_keys
	{
		int keyUp = KEYBOARD::KEY::UP;
		int keyDown = KEYBOARD::KEY::DOWN;
		int keyLeft = KEYBOARD::KEY::LEFT;
		int keyRight = KEYBOARD::KEY::RIGHT;
		int keyInsert = KEYBOARD::KEY::INS;
	};

	struct input_text : surface, active_element, input_text_appearance
	{
	private:
		text m_text;
		console_string m_str;
		vec2i m_cursor_pos = { 0,0 };
		int m_cursor_pos_in_txt = 0;
		time_frame m_cursor_blink;

		bool m_redraw_needed = true;
		bool m_insert_mode = true;
		bool m_confidential_mode = false;
		bool m_blink = true;

		input_text_keys m_keys;

		void updateCursorPos()
		{
			vec2i old_pos = m_cursor_pos;

			if (m_text.getText().size() > 0)
			{
				auto getRelativePos = [&]()
				{
					vec2i pos = m_text.getSymbolPos(m_cursor_pos_in_txt);
					pos.y -= m_text.getLine();

					return pos;
				};

				m_cursor_pos = getRelativePos();

				if (m_cursor_pos.y >= getSize().y)
				{
					int c = m_cursor_pos.y - (getSize().y-1);

					m_text.goToLine(m_text.getLine()+c);
					m_cursor_pos = getRelativePos();
				}
				else if (m_cursor_pos.y < 0)
				{
					int c = m_cursor_pos.y * -1;

					m_text.goToLine(m_text.getLine()-c);
					m_cursor_pos = getRelativePos();
				}

			}
			else { m_cursor_pos = { 0,0 }; }

			if (m_cursor_pos != old_pos) { m_redraw_needed = true; }
		}

		void updateText()
		{
			console_string s;
			
			switch (m_confidential_mode)
			{
			case false:
				s = m_str;
				break;
			case true:
				s.resize(m_str.size());
				for (int i = 0;i < s.size(); i++)
				{
					s[i] = '*';
				}
			}
			
			s += " ";

			if (m_cursor_pos_in_txt > s.size()) { m_cursor_pos_in_txt = s.size(); }

			m_text.setText(s);
		}
		
		void moveCursorRight(unsigned int n = 1)
		{
			if (m_cursor_pos_in_txt + n <= m_str.size()) 
			{ 
				m_cursor_pos_in_txt += n; 
				m_redraw_needed = true;
			}
			else
			{
				m_cursor_pos_in_txt = m_str.size();
				m_redraw_needed = true;
			}
		}
		void moveCursorLeft(unsigned int n = 1)
		{
			if (m_cursor_pos_in_txt - (int)n >= 0) //without casting "n" to signed "m_cursor_pos_in_txt" is implicitly casted to unsigned and condition is always true 
			{ 
				m_cursor_pos_in_txt -= n; 
				m_redraw_needed = true;
			}
			else
			{
				m_cursor_pos_in_txt = 0;
				m_redraw_needed = true;
			}
		}
		void moveCursorUp(unsigned int n = 1)
		{
			if (m_str.size() > 0)
			{
				vec2i c_pos = m_text.getSymbolPos(m_cursor_pos_in_txt);

				if (c_pos.y > 0)
				{
					if (c_pos.y < n)
					{
						n = c_pos.y;
					}

					for (int l_pos = m_cursor_pos_in_txt - 1; l_pos >= 0; --l_pos)
					{
						if (m_text.getSymbolPos(l_pos).y == c_pos.y - n && m_text.getSymbolPos(l_pos).x <= c_pos.x)
						{
							m_cursor_pos_in_txt = l_pos;
							m_redraw_needed = true;
							break;
						}
					}
				}
			}
		}
		void moveCursorDown(unsigned int n = 1)
		{
			if (m_str.size() > 0)
			{
				vec2i c_pos = m_text.getSymbolPos(m_cursor_pos_in_txt);

				if (c_pos.y < m_text.getNumberOfLines()-1)
				{
					if (c_pos.y >= m_text.getNumberOfLines() - n)
					{
						n = m_text.getNumberOfLines() - c_pos.y - 1;
					}

					for (int l_pos = m_cursor_pos_in_txt + 1; l_pos <= m_str.size(); ++l_pos)
					{
						if (m_text.getSymbolPos(l_pos).y == c_pos.y + n && (m_text.getSymbolPos(l_pos).x == c_pos.x || l_pos == m_str.size()))
						{
							m_cursor_pos_in_txt = l_pos;
							m_redraw_needed = true;
							break;
						}
						else if (m_text.getSymbolPos(l_pos).y > c_pos.y + n)
						{
							m_cursor_pos_in_txt = l_pos - 1;
							m_redraw_needed = true;
							break;
						}
					}
				}
			}
		}

		void fill()
		{		
			updateCursorPos();

			if (m_redraw_needed)
			{
				makeTransparent();
				insertSurface(m_text, false);
			}

			if (isActive() && (m_cursor_blink.isEnd(false) || m_redraw_needed))
			{
				if (m_blink)
				{
					if (m_insert_mode) { setSymbolAt(m_insert_cursor, m_cursor_pos); }
					else { setSymbolAt(m_overtype_cursor, m_cursor_pos); }
				}
				else
				{
					setSymbolAt(m_text.getSymbolAt(m_cursor_pos), m_cursor_pos);
				}

				if (m_cursor_blink.isEnd(true)) { m_blink = !m_blink; }
			}

			m_redraw_needed = false;
		}

		void resizeAction() override { m_redraw_needed = true; }
		void updateAction() override { update(); }
		void drawAction() override { fill(); }

		void activationAction() override
		{
			m_text.activate();
			m_redraw_needed = true;
		}
		void disactivationAction() override
		{
			m_text.disactivate();
			m_redraw_needed = true;
		}

		void setAppearanceAction() override
		{
			m_text.setAppearance(m_text_appearance);
			m_redraw_needed = true;
		}
	public:
		input_text() : m_cursor_blink(std::chrono::milliseconds(500))
		{
			m_text.setSizeInfo({ {0,0},{100,100} });
			m_text.useDensePunctuation(false);
			m_text.usePreparedText(false);
			m_text.setAppearance(m_text_appearance);
		}

		console_string getText() { return m_str; }

		void setKeys(input_text_keys keys) { m_keys = keys; }
		input_text_keys getKeys() { return m_keys; }


		void useConfidentialMode(bool use)
		{
			m_confidential_mode = use;
			updateText();
			m_redraw_needed = true;
		}
		bool isUsingConfidentialMode() { return m_confidential_mode; }

		void update()
		{
			if (isActive())
			{
				console_string input = KEYBOARD::getInputAsString();

				if (input.size() > 0)
				{
					if (m_insert_mode)
					{
						for (int i = 0; i < input.size(); i++)
						{
							m_str.insert(m_str.begin() + m_cursor_pos_in_txt + i, input[i]);
						}
					}
					else//overtype
					{
						for (int i = 0; i < input.size(); i++)
						{
							if (i + m_cursor_pos_in_txt < m_str.size())
							{
								m_str[i + m_cursor_pos_in_txt] = input[i];
							}
							else
							{
								m_str.push_back(input[i]);
							}
						}	
					}
					moveCursorRight(input.size());
					updateText();
				}


				if (KEYBOARD::isKeyPressed(KEYBOARD::KEY::BACKSPACE))
				{
					int erase_c = KEYBOARD::isKeyPressed(KEYBOARD::KEY::BACKSPACE);
					if (erase_c > m_cursor_pos_in_txt) { erase_c = m_cursor_pos_in_txt; }

					m_str.erase(m_str.begin() + m_cursor_pos_in_txt - erase_c, m_str.begin() + m_cursor_pos_in_txt);

					moveCursorLeft(erase_c);
					updateText();
				}

				if (KEYBOARD::isKeyPressed(m_keys.keyLeft)) { moveCursorLeft(KEYBOARD::isKeyPressed(m_keys.keyLeft)); }
				if (KEYBOARD::isKeyPressed(m_keys.keyRight)) { moveCursorRight(KEYBOARD::isKeyPressed(m_keys.keyRight)); }
				if (KEYBOARD::isKeyPressed(m_keys.keyUp)) { moveCursorUp(KEYBOARD::isKeyPressed(m_keys.keyUp)); }
				if (KEYBOARD::isKeyPressed(m_keys.keyDown)) { moveCursorDown(KEYBOARD::isKeyPressed(m_keys.keyDown)); }

				if (KEYBOARD::isKeyPressed(m_keys.keyInsert)) { m_insert_mode = !m_insert_mode; }
			}
		}
	};
}
