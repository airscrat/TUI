#pragma once
#include "tui_console.h"
#include "tui_appearance.h"
#include "tui_text_utils.h"
#include "tui_scroll.h"

namespace tui
{
	struct text_appearance : appearance
	{
	protected:
		scroll_appearance text_scroll_appearance;
	public:
		text_appearance() : text_appearance(scroll_appearance()) {}
		text_appearance(scroll_appearance scroll) : text_scroll_appearance(scroll) {}
		void setColor(color Color) override
		{
			text_scroll_appearance.setColor(Color);
			setAppearanceAction();
		}
		void setAppearance(text_appearance appearance)
		{
			*this = appearance;
			setAppearanceAction();
		}
		text_appearance getAppearance() { return *this; }

		void setScrollAppearance(scroll_appearance scroll)
		{
			text_scroll_appearance = scroll;
			setAppearanceAction();
		}
		scroll_appearance getScrollAppearance() { return text_scroll_appearance; }
	};

	struct text : surface, text_appearance, active_element
	{
	private:
		surface m_text;
		scroll<DIRECTION::VERTICAL> m_scroll;

		console_string m_unprepared_text;
		console_string m_prepared_text;

		std::vector<vec2i> m_symbolPos;

		bool m_use_prepared_text = true;
		bool m_use_dense_punctuation = false;
		bool m_use_control_characters = true;
		bool m_display_scroll = true;

		void fill()
		{
			m_text.makeTransparent();

			for (int y = 0; y < m_text.getSize().y; y++)
			{
				for (int x = 0; x < m_text.getSize().x; x++)
				{
					if (m_text.getSize().x * m_scroll.getHandlePosition() + y * m_text.getSize().x + x < m_prepared_text.size())
					{
						m_text[x][y] = m_prepared_text[m_text.getSize().x * m_scroll.getHandlePosition() + y * m_text.getSize().x + x];
					}
				}
			}

			makeTransparent();
			insertSurface(m_text);
			if (m_display_scroll && m_scroll.isNeeded()) { insertSurface(m_scroll); }
		}
		void prepareText()
		{
			console_string prepared;

			m_symbolPos.resize(m_unprepared_text.size());

			auto usePrepared = [&]()
			{
				return m_use_prepared_text && getSize().x > 2;
			};

			int pos = 0;
			for (int i = 0; i < m_unprepared_text.size(); i++)
			{
				int pos_in_line = pos % m_text.getSize().x;

				m_symbolPos[i] = { pos_in_line, (int)floor((float)pos / m_text.getSize().x) };

				if (IsControl(m_unprepared_text[i].getFirstChar()))
				{
					if (m_use_control_characters)
					{
						if (m_unprepared_text[i].getFirstChar() == '\n')
						{
							for (int i = 0; i < (m_text.getSize().x - pos_in_line); i++)
							{
								prepared.push_back(' ');
							}
							pos += m_text.getSize().x - pos_in_line;
						}
					}
					continue;
				}

				if (usePrepared()
					&& pos_in_line == m_text.getSize().x - 1
					&& i + 1 < m_unprepared_text.size()
					&& m_unprepared_text[i].getFirstChar() != (U' ')
					&& m_unprepared_text[i + 1].getFirstChar() != (U' ')
					&& !IsControl(m_unprepared_text[i + 1].getFirstChar())
					&& (m_use_dense_punctuation ? !IsPunctuation(m_unprepared_text[i]) : true)
					&& (m_use_dense_punctuation ? !IsPunctuation(m_unprepared_text[i + 1]) : true)
					)
				{
					prepared << m_unprepared_text[i - 1].getColor();
					if (m_unprepared_text[i - 1].getFirstChar() != U' ')
					{
						prepared << "-";
					}
					else { prepared << " "; }
					pos++;
				}
				if (usePrepared() && pos_in_line == 0 && m_unprepared_text[i].getFirstChar() == U' ') { continue; }//omit space at start of line

				prepared.push_back(m_unprepared_text[i]);
				pos++;
			}

			m_prepared_text = prepared;
		}

		void adjustSizes()
		{
			m_text.setSize({ {0,0}, {100,100} });
			updateSurfaceSize(m_text);
			prepareText();
			if (m_display_scroll)
			{
				if (getNumberOfLines() > m_text.getSize().y)
				{
					m_text.setSize({ {-1,0}, {100,100} });
					updateSurfaceSize(m_text);
					prepareText();
				}
			}
			m_scroll.setContentLength(getNumberOfLines());
		}

		void immobilizeScroll(bool immobilize)
		{
			switch (immobilize)
			{
			case true:
				m_scroll.keyDown = -1;
				m_scroll.keyUp = -1;
				m_scroll.keyPageDown = -1;
				m_scroll.keyPageUp = -1;
				break;
			case false:
				m_scroll.keyDown = keyDown;
				m_scroll.keyUp = keyUp;
				m_scroll.keyPageDown = keyPageDown;
				m_scroll.keyPageUp = keyPageUp;
				break;
			}
		}

	public:
		int keyUp = KEYBOARD::KEY::UP;
		int keyDown = KEYBOARD::KEY::DOWN;
		int keyPageUp = KEYBOARD::KEY::PGUP;
		int keyPageDown = KEYBOARD::KEY::PGDN;
		
		text() : text({ {1,1} }) {}
		text(surface_size size) : text(size, U"") {}
		text(surface_size size, console_string txt) : m_scroll({ 0, 100 })
		{
			setSize(size);
			m_text.setSize({ {-1,0}, {100,100} });

			m_scroll.setPosition(position({ 0,0 }, { 0,0 }, { POSITION::HORIZONTAL::RIGHT, POSITION::VERTICAL::TOP }));
			setText(txt);

			setAppearanceAction();
		}

		vec2i getSymbolPos(int i)
		{
			return m_symbolPos[i];
		}

		void setText(console_string txt)
		{
			m_unprepared_text = txt;
			adjustSizes();
			fill();
		}

		console_string getText() { return m_unprepared_text; }
		int getNumberOfLines()
		{
			return ceil(m_prepared_text.size() / (m_text.getSize().x * 1.f));
		}

		void goToLine(int line)
		{
			m_scroll.setHandlePosition(line);
			fill();
		}

		//return current line number
		int getLine() { return m_scroll.getHandlePosition(); }

		void lineUp()
		{
			m_scroll.setHandlePosition(m_scroll.getHandlePosition() - 1);
			fill();
		}
		void lineDown()
		{
			m_scroll.setHandlePosition(m_scroll.getHandlePosition() + 1);
			fill();
		}
		void pageUp()
		{
			m_scroll.setHandlePosition(m_scroll.getHandlePosition() - getSize().y);
			fill();
		}
		void pageDown()
		{
			m_scroll.setHandlePosition(m_scroll.getHandlePosition() + getSize().y);
			fill();
		}

		void usePreparedText(bool use)
		{
			m_use_prepared_text = use;
			adjustSizes();
			fill();
		}
		bool isUsingPreparedText() { return m_use_prepared_text; }

		void useDensePunctuation(bool use)
		{
			m_use_dense_punctuation = use;
			adjustSizes();
			fill();
		}
		bool isUsingDensePunctuation() { return m_use_dense_punctuation; }

		void useControlCharacters(bool use)
		{
			m_use_control_characters = use;
			adjustSizes();
			fill();
		}
		bool isUsingControlCharacters() { return m_use_control_characters; }

		void useScroll(bool display)
		{
			m_display_scroll = display;
			adjustSizes();
			fill();
		}
		//is displaying scroll if needed
		bool isUsingScroll() { return m_display_scroll; }

		//is displaying scroll currently
		bool isDisplayingScroll() { return m_display_scroll && m_scroll.isNeeded(); }

		void resizeToText(int max_width)
		{
			if (max_width <= 0)
			{
				bool use_c_char = m_use_control_characters;
				m_use_control_characters = false;//temporarily disable control characters

				setSize({ {(int)m_unprepared_text.size(), 1} });
				adjustSizes();
				/*
				with control characters disabled one line prepared text could be shorter than unprepared text
				e.g. space at start of line will be removed and all control characters will be removed
				*/
				setSize({ {(int)m_prepared_text.size(), 1} });

				m_use_control_characters = use_c_char;
			}
			else
			{
				bool display_scroll = m_display_scroll;
				m_display_scroll = false;

				setSize({ {max_width, 1} });
				prepareText();

				setSize({ {max_width, (int)ceil(1.f * m_prepared_text.size() / max_width)} });
				fill();

				m_display_scroll = display_scroll;
			}
		}
		void resizeToText() { resizeToText(0); }

		void update()
		{
			immobilizeScroll(false);

			int pos = m_scroll.getHandlePosition();

			m_scroll.update();
			
			//immobilize scroll beacuse draw() function may be called outside this function
			immobilizeScroll(true);

			if (pos != m_scroll.getHandlePosition()) { fill(); }
		}

		void resizeAction() override
		{
			updateSurfaceSize(m_scroll);
			adjustSizes();
			fill();
		}
		void updateAction() override { update(); }

		void activationAction() override { m_scroll.activate(); }
		void disactivationAction() override { m_scroll.disactivate(); }

		void setAppearanceAction() override 
		{ 
			m_scroll.setAppearance(text_scroll_appearance);
			fill();
		}

	};
}