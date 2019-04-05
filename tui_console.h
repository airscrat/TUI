#pragma once
#include <vector>

#ifdef TUI_TARGET_SYSTEM_WINDOWS
#define UNICODE
	#include <windows.h>
#endif

#ifdef TUI_TARGET_SYSTEM_LINUX
	#include <sys/ioctl.h>
	#include <unistd.h>
#endif

#include "tui_utils.h"
#include "tui_enums.h"

//#ifdef  TUI_TARGET_SYSTEM_WINDOWS
	#include "tui_input.h"
//#endif



namespace tui
{
	struct active_element
	{
	private:
		bool active = false;
	public:
		virtual void activation_action() {}
		virtual void disactivation_action() {}

		void activate() 
		{ 
			active = true; 
			activation_action();
		}
		void disactivate() 
		{ 
			active = false; 
			disactivation_action();
		}
		bool isActive() { return active; }
	};

	struct navigation_group : active_element
	{
	private:
		std::vector<int> m_key_combo_next;
		std::vector<active_element*> m_elements;

		bool m_was_next_pressed = false;

		int m_selected = -1;

		time_frame m_time_limit;


		void disactivateAll()
		{
			for (int i = 0; i < m_elements.size(); i++)
			{
				m_elements[i]->disactivate();
			}
		}

	public:

		navigation_group() : m_time_limit(std::chrono::milliseconds(300)) {}

		void setKeyComboNext(std::vector<int> combo)
		{
			m_key_combo_next = combo;
		}
		void addElement(active_element &element)
		{
			m_elements.push_back(&element);
		}

		void update()
		{
			if (isActive())
			{

				int act = 0;

				for (int i = 0; i < m_key_combo_next.size(); i++)
				{
					//if (KEYBOARD::isKeyPressed(m_key_combo_next[i], TUI_BUFFERED_INPUT))
					{
						act++;
					}
				}
				if (act == m_key_combo_next.size())
				{
					m_was_next_pressed = true;
				}

				if (m_time_limit.isEnd())
				{
					

					if (m_was_next_pressed)
					{
						disactivateAll();

						if (m_selected < (int)m_elements.size() - 1) { m_selected++; }
						else { m_selected = 0; }
						m_elements[m_selected]->activate();

						m_was_next_pressed = false;
					}
				}
			}
		}

		void disactivation_action() { disactivateAll(); }
		void activation_action() 
		{
			disactivateAll();
			m_selected = 0;
			if (m_elements.size() > 0)
			{
				m_elements[0]->activate();
			}
		}

	};

	struct surface
	{
		friend class group;

		private:		
			std::vector<std::vector<console_char>> m_chars;
			position m_position;
			//vec2i percentageSize;
			surface_size size_info;
			//int sizeType;

			void resize(vec2i size)
			{
				if (size.x != getSize().x || size.y != getSize().y)
				{
					if (size.x > 0 && size.y > 0)
					{
						m_chars.resize(size.x);
						for (int i = 0; i < m_chars.size(); i++)
						{
							m_chars[i].resize(size.y);
						}
					}
					else
					{
						m_chars.resize(1);
						m_chars[0].resize(1);
					}
					makeTransparent();
					resize_action();
				}
			}
			
		protected:
			virtual void resize_action() {}
		public:	
			surface()
			{
				m_chars.resize(1);
				m_chars[0].resize(1);
			}

			virtual void draw_action() {}

			//void setSizeType(int type) { sizeType = type; }

			void setChar(console_char character, vec2i position) { m_chars[position.x][position.y] = character; }
			console_char getchar(vec2i position) { return m_chars[position.x][position.y]; }
			void setPosition(position pos) { m_position = pos; }

			void move(vec2i offset)
			{
				vec2i act_pos = getPosition().getOffset();			
				m_position.setOffset(vec2i(act_pos.x + offset.x, act_pos.y + offset.y));
			}

			/*void modifySize(vec2i size)
			{
				percentageSize = size;

				resize(size);
			}*/
			
			void setSize(surface_size size)
			{
				size_info = size;

				//resize(size.getIntegerSize());

				vec2i perc_size =size.getPercentagesize();
				vec2i int_size = size.getIntegerSize();

				int x = (perc_size.x / 100.f) * getSize().x + int_size.x;
				int y = (perc_size.y / 100.f) * getSize().y + int_size.y;

				//if (x < 1) { x = 1; }
				//if (y < 1) { y = 1; }

				resize({ x,y });

			}

			void updateSurfaceSize(surface &obj)
			{
				vec2i perc_size = obj.getSizeInfo().getPercentagesize();
				vec2i int_size = obj.getSizeInfo().getIntegerSize();

				int x = (perc_size.x / 100.f) * getSize().x + int_size.x;
				int y = (perc_size.y / 100.f) * getSize().y + int_size.y;

				obj.resize({x,y});

				/*vec2i perc_size = obj.getPercentageSize();
				switch (obj.getSizeType())
				{
				case SIZE::CONSTANT:
					//	obj.resize(obj.getSize());
					break;
				case SIZE::PERCENTAGE:
					obj.resize(vec2i((perc_size.x / 100.f) * getSize().x, (perc_size.y / 100.f) * getSize().y));
					break;
				case SIZE::PERCENTAGE_X:
					obj.resize(vec2i((perc_size.x / 100.f) * getSize().x, obj.getSize().y));
					break;
				case SIZE::PERCENTAGE_Y:
					obj.resize(vec2i(obj.getSize().x, (perc_size.y / 100.f) * getSize().y));
					break;
				}*/
			}

			void insertSurface(surface &obj)
			{
				updateSurfaceSize(obj);
				
				obj.draw_action();


				int x_origin = getSize().x * (obj.getPosition().getRelativePoint().x / 100.f) - obj.getSize().x * (obj.getPosition().getRelativePoint().x / 100.f);
				x_origin += obj.getPosition().getOffset().x;
				x_origin += obj.getPosition().getPercentageOffset().x * getSize().x/ 100.f;

				int y_origin = getSize().y * (obj.getPosition().getRelativePoint().y / 100.f) - obj.getSize().y * (obj.getPosition().getRelativePoint().y / 100.f);
				y_origin += obj.getPosition().getOffset().y;
				y_origin += obj.getPosition().getPercentageOffset().y * getSize().y /100.f;

				for (int i = 0; i < obj.getSize().x; i++)
				{
					for (int j = 0; j < obj.getSize().y; j++)
					{
						if (x_origin + i < getSize().x
							&& y_origin + j < getSize().y
							&& x_origin + i >= 0
							&& y_origin + j >= 0
							&& obj.getChar(vec2i(i, j)).getChar() != TRANSPARENT)
						{
							m_chars[x_origin + i][y_origin + j] = obj.getChar(vec2i(i, j));
						}
					}
				}
			}

			void makeTransparent()
			{
				for (int i = 0; i < getSize().x; i++)
				{
					for (int j = 0; j < getSize().y; j++)
					{
						m_chars[i][j] = TRANSPARENT;
					}
				}
			}
			void makeBlank()
			{
				for (int i = 0; i < getSize().x; i++)
				{
					for (int j = 0; j < getSize().y; j++)
					{
						m_chars[i][j] = BLANKSYMBOL;
					}
				}
			}

//int getSizeType() { return sizeType; }
	//		vec2i getPercentageSize() { return percentageSize; }
			position getPosition() { return m_position; }
			vec2i getSize() { return vec2i(m_chars.size(), m_chars[0].size()); }
			surface_size getSizeInfo() { return size_info; }
			console_char getChar(vec2i position) { return m_chars[position.x][position.y]; }
			
		
	};

	struct group : surface
	{
		std::vector<surface*> m_surfaces;


		group(surface_size size)
		{
			setSize(size);
		}


		void addSurface(surface &surf)
		{
			m_surfaces.push_back(&surf);
		}

		void removeSurface(surface &surf)
		{
			for (int i = 0; i < m_surfaces.size(); i++)
			{
				if (m_surfaces[i] == &surf)
				{
					m_surfaces.erase(m_surfaces.begin() + i);
				}
			}
		}

		void draw_action()
		{
			makeTransparent();

			for (int i = 0; i < m_surfaces.size(); i++)
			{
				insertSurface(*m_surfaces[i]);
			}
		}

		void resize_action()
		{

		}
	};



	//buffer
	struct console_buffer
	{
	protected:
		surface m_buffer;

		console_buffer() {}

		void resize(vec2i size) { m_buffer.setSize(size); }

	public:
		vec2i getSize() { return m_buffer.getSize(); }

		console_char getChar(vec2i position) { return m_buffer.getChar(position); }

		void draw(surface &surf) { m_buffer.insertSurface(surf); }

		void clear_buf() { m_buffer.makeBlank(); }
	};


	//console
	struct console : console_buffer
	{
	private:
#ifdef TUI_TARGET_SYSTEM_WINDOWS
		HANDLE m_console_handle;
#endif
		vec2i m_last_size;
		bool m_resized;
		time_frame m_fps_control;

	public:
		console() : m_fps_control(std::chrono::milliseconds(1000) / 30)
		{
#ifdef  TUI_TARGET_SYSTEM_WINDOWS
			//system("chcp 437");
			system("chcp 10000");
			m_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
			//SetConsoleDisplayMode(GetStdHandle(STD_OUTPUT_HANDLE), CONSOLE_FULLSCREEN_MODE, 0);
			//SetConsoleMode(m_console_handle, 0);
#endif

			updateSize();
			hidePrompt();
		}


		bool was_resized() { return m_resized; }

		void setFPSlimit(int fps)
		{
			m_fps_control.setFrameTime(std::chrono::milliseconds(1000) / fps);
		}

		void clear()
		{
			updateSize();
			clear_buf();
		}

		void display()
		{
			m_fps_control.sleepUntilEnd();

			KEYBOARD::buffer.clear();

#ifdef  TUI_TARGET_SYSTEM_WINDOWS
			CONSOLE_SCREEN_BUFFER_INFO buffer_info;
			GetConsoleScreenBufferInfo(m_console_handle, &buffer_info);
			vec2i console_size(buffer_info.dwSize.X, buffer_info.dwSize.Y);

			std::vector<WORD> temp_attr;
			std::vector<wchar_t> temp_char;

			for (int i = 0; i < getSize().y; i++)
			{
				for (int j = 0; j < console_size.x; j++)
				{
					if (j < getSize().x)
					{
						temp_attr.push_back(m_buffer.getChar(vec2i(j, i)).getColor().getRGBIColor());
						temp_char.push_back(m_buffer.getChar(vec2i(j, i)).getChar());
					}
					else
					{
						temp_attr.push_back(console_color().getRGBIColor());
						temp_char.push_back(wchar_t());
					}
				}
			}
			COORD coord = { 0, 0 };
			DWORD useless = 0;
			SetConsoleCursorPosition(m_console_handle, coord); // w/o 10x slower

			WriteConsoleOutputAttribute(m_console_handle, temp_attr.data(), console_size.x*getSize().y, coord, &useless);
			WriteConsoleOutputCharacterW(m_console_handle, temp_char.data(), console_size.x*getSize().y, coord, &useless);


		
			SetConsoleCursorPosition(m_console_handle, coord);
#endif
			setGlobalColor(console_color(COLOR::WHITE, COLOR::BLACK));
			hidePrompt();
		}

		void setTitle(std::string title) 
		{
#ifdef  TUI_TARGET_SYSTEM_WINDOWS
			SetConsoleTitleA(title.c_str()); 
#endif
		}

	private:
		void updateSize()
		{
			vec2i console_size;

#ifdef  TUI_TARGET_SYSTEM_WINDOWS
			CONSOLE_SCREEN_BUFFER_INFO buffer_info;
			GetConsoleScreenBufferInfo(m_console_handle, &buffer_info);
			
			console_size.x = buffer_info.srWindow.Right - buffer_info.srWindow.Left + 1;
			console_size.y = buffer_info.srWindow.Bottom - buffer_info.srWindow.Top + 1;
#endif

#ifdef TUI_TARGET_SYSTEM_LINUX
			winsize w;
			ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

			console_size.x = w.ws_col;
			console_size.y = w.ws_row;
#endif

			if (console_size.x != m_last_size.x || console_size.y != m_last_size.y)
			{
				resize(console_size);
				m_resized = true;
			}
			else { m_resized = false; }

			m_last_size = console_size;
		}

		void setGlobalColor(console_color color) 
		{
#ifdef TUI_TARGET_SYSTEM_WINDOWS
			SetConsoleTextAttribute(m_console_handle, color.getRGBIColor()); 
#endif

#ifdef TUI_TARGET_SYSTEM_LINUX
			std::cout << color.getEscapeCode();
#endif
		}

		void hidePrompt()
		{
#ifdef TUI_TARGET_SYSTEM_WINDOWS
			CONSOLE_CURSOR_INFO cursor_info;
			SetConsoleCursorInfo(m_console_handle, &cursor_info);
			cursor_info.bVisible = false;
			cursor_info.dwSize = 1;

			SetConsoleCursorInfo(m_console_handle, &cursor_info);
#endif

#ifdef TUI_TARGET_SYSTEM_LINUX
			std::cout << "\033[?25l";
#endif
		}
	};
}