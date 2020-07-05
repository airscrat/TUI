#pragma once
#include "tui_surface.h"

#include <vector>


namespace tui
{
	struct group : surface
	{
	private:
		std::vector<surface*> m_surfaces;

		void drawAction() override
		{
			makeTransparent();

			for (int i = 0; i < m_surfaces.size(); i++)
			{
				insertSurface(*m_surfaces[i]);
			}
		}
	public:
		group(surface_size size = { {1,1},{0,0} })
		{
			setSizeInfo(size);
		}

		void setSurfaces(std::vector<surface*> surfaces)
		{
			m_surfaces = surfaces;
		}
		void addSurface(surface& surf)
		{
			m_surfaces.push_back(&surf);
		}
		void removeSurface(surface& surf)
		{
			for (int i = 0; i < m_surfaces.size(); i++)
			{
				if (m_surfaces[i] == &surf)
				{
					m_surfaces.erase(m_surfaces.begin() + i);
				}
			}
		}
		void removeAll()
		{
			m_surfaces.clear();
		}
	};
}