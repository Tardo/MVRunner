/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */
#ifndef ENGINE_QUADTREE_H
#define ENGINE_QUADTREE_H
#include <SFML/Graphics/Rect.hpp>

#include <list>

template<class T>
class CQuadTree final
{
	static const unsigned int QT_NODE_CAPACITY = 1;

private:

	struct CQTItem final
	{
		sf::Vector2f m_Pos;
		T m_Item;
	} m_aItems[QT_NODE_CAPACITY];

	sf::FloatRect m_Bounds;
	unsigned int m_NumItems;
	bool m_Subdivided;

	CQuadTree *m_pParent;
	CQuadTree *m_pNW;
	CQuadTree *m_pNE;
	CQuadTree *m_pSW;
	CQuadTree *m_pSE;

	void subdivide() noexcept
	{
		const float halfW = m_Bounds.width/2.0f;
		const float halfH = m_Bounds.height/2.0f;
		m_pNW = new CQuadTree(sf::FloatRect(m_Bounds.left, m_Bounds.top, halfW, halfH), this);
		m_pNE = new CQuadTree(sf::FloatRect(m_Bounds.left+halfW, m_Bounds.top, halfW, halfH), this);
		m_pSW = new CQuadTree(sf::FloatRect(m_Bounds.left, m_Bounds.top+halfH, halfW, halfH), this);
		m_pSE = new CQuadTree(sf::FloatRect(m_Bounds.left+halfW, m_Bounds.top+halfH, halfW, halfH), this);
		m_Subdivided = true;

		for (int p=m_NumItems-1; p>=0; --p)
			insert(m_aItems[p].m_Pos, m_aItems[p].m_Item);
		m_NumItems = 0;
	}

public:
	CQuadTree(const sf::FloatRect &bounds, CQuadTree *pParent = nullptr) noexcept
	{
		m_Bounds = bounds;
		m_NumItems = 0;
		m_Subdivided = false;

		m_pParent = pParent;
		m_pNW = nullptr;
		m_pNE = nullptr;
		m_pSW = nullptr;
		m_pSE = nullptr;
	}
	~CQuadTree() noexcept
	{
		if (m_Subdivided)
		{
			delete m_pNW;
			delete m_pNE;
			delete m_pSW;
			delete m_pSE;
		}
	}


	bool insert(const sf::Vector2f &Pos, const T &Item) noexcept
	{
		if (!m_Bounds.contains(Pos))
			return false;

		if (m_Subdivided)
		{
			return m_pNW->insert(Pos, Item) ||
					m_pNE->insert(Pos, Item) ||
					m_pSW->insert(Pos, Item) ||
					m_pSE->insert(Pos, Item);
		} else
		{
			if (m_NumItems == QT_NODE_CAPACITY)
			{
				subdivide();
				insert(Pos, Item);
			}
			else
			{
				m_aItems[m_NumItems].m_Pos = Pos;
				m_aItems[m_NumItems].m_Item = Item;
				++m_NumItems;
			}

			return true;
		}

		return false;
	}

	std::list<T> queryAABB(const sf::FloatRect &bounds) noexcept
	{
		std::list<T> pointsInRange;
		if (!bounds.intersects(m_Bounds))
			return pointsInRange; // empty list

		if (!m_Subdivided)
		{
			for (int p = m_NumItems-1; p>=0; --p)
				pointsInRange.push_back(m_aItems[p].m_Item);
		} else
		{
			std::list<T> vQueryList = m_pNW->queryAABB(bounds);
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
			vQueryList = m_pNE->queryAABB(bounds);
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
			vQueryList = m_pSW->queryAABB(bounds);
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
			vQueryList = m_pSE->queryAABB(bounds);
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
		}

		return pointsInRange;
	}

	const sf::FloatRect& getBounds() const noexcept { return m_Bounds; }

	std::list<T> queryAll() noexcept
	{
		std::list<T> pointsInRange;

		if (!m_Subdivided)
		{
			for (int p = m_NumItems-1; p>=0; --p)
				pointsInRange.push_back(m_aItems[p].m_Item);
		} else
		{
			std::list<T> vQueryList = m_pNW->queryAll();
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
			vQueryList = m_pNE->queryAll();
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
			vQueryList = m_pSW->queryAll();
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
			vQueryList = m_pSE->queryAll();
			pointsInRange.insert(pointsInRange.end(), vQueryList.begin(), vQueryList.end());
		}

		return pointsInRange;
	}

	void clear() noexcept
	{
		m_NumItems = 0;

		if (m_pNW)
		{
			m_pNW->clear();
			m_pNE->clear();
			m_pSW->clear();
			m_pSE->clear();
			delete m_pNW;
			delete m_pNE;
			delete m_pSW;
			delete m_pSE;
			m_pNW = nullptr;
			m_pNE = nullptr;
			m_pSW = nullptr;
			m_pSE = nullptr;
			m_pParent = nullptr;
		}
	}
};

#endif // ENGINE_QUADTREE_H
