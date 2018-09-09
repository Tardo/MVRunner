/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#ifndef H_GAME_UI
#define H_GAME_UI


class CListItem
{
public:
	CListItem()
	{
		m_Visible = true;
		m_Selected = false;
	}

	sf::FloatRect m_Bounds;
	bool m_Visible;
	bool m_Selected;
};

class CUI final
{
	static const sf::Vector2f SCROLL_SIZE;
public:
	enum
	{
		ALIGN_LEFT=0,
		ALIGN_RIGHT,
		ALIGN_CENTER,

		TEXT_SIZE_SMALL=16,
		TEXT_SIZE_NORMAL=32,
		TEXT_SIZE_HEADER=80,

		CONTROL_NONE=0,
		CONTROL_BUTTON,
		CONTROL_CHECK,
		CONTROL_EDIT,
		CONTROL_LIST,
	};


	CUI(const sf::Vector2u &screenSize, const sf::Font &font) noexcept;
	virtual ~CUI() noexcept;

	void processEvent(const sf::Event &event) noexcept;

	bool doButtonLogic(const void *pId, const sf::FloatRect &bounds) noexcept;
	bool doButton(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const char *pText, const sf::FloatRect &bounds, unsigned int fontSize, int align = 0) noexcept;
	bool doCheck(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const sf::FloatRect &bounds, bool *pSelected, const char *pText, unsigned int fontSize, const sf::Color &color, int align = 0) noexcept;
	bool doEdit(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const sf::FloatRect &bounds, const char *pText, unsigned int fontSize, const sf::Color &color, unsigned int maxLength) noexcept;


	void doListInit(sf::RenderTarget& target, sf::RenderStates states, const void *pId, const sf::FloatRect &bounds, int numItems, int *pSelected, float HeightPerRow, float *pScrollOffset) noexcept;
	CListItem doNextListItem(const void *pId, const void *pRowId) noexcept;
	void doListEnd(sf::RenderTarget& target, sf::RenderStates states, const void *pId) noexcept;

	void doLabel(sf::RenderTarget& target, sf::RenderStates states, const char*pText, const sf::FloatRect &bounds, const sf::Color &color, unsigned int fontSize, int align = 0) const noexcept;
	void doBox(sf::RenderTarget& target, sf::RenderStates states, const sf::FloatRect &bounds, const sf::Color &color, float outline=0.0f, const sf::Color &outlineColor=sf::Color::Transparent) const noexcept;

	void clipEnable(const sf::RenderTarget &target, sf::IntRect bounds) const noexcept;
	void clipDisable() const noexcept;

	const sf::Vector2i& getMousePos() const { return m_MousePosition; }
	bool isMouseLeftClicked() const { return m_MouseLeftClicked; }
	bool isMouseRightClicked() const { return m_MouseRightClicked; }

	static inline void splitVMid(sf::FloatRect *pBoundsA, sf::FloatRect *pBoundsB) noexcept
	{
		pBoundsA->width /= 2.0f;
		*pBoundsB = *pBoundsA;
		pBoundsB->left += pBoundsA->width;

	}
	static inline void splitHMid(sf::FloatRect *pBoundsA, sf::FloatRect *pBoundsB)
	{
		pBoundsA->height /= 2.0f;
		*pBoundsB = *pBoundsA;
		pBoundsB->top += pBoundsA->height;
	}
	static inline void splitV(sf::FloatRect *pBoundsA, sf::FloatRect *pBoundsB, float width)
	{
		const float cw = pBoundsA->width - width;
		pBoundsA->width = width;
		*pBoundsB = *pBoundsA;
		pBoundsB->left += pBoundsA->width;
		pBoundsB->width = cw;
	}
	static inline void splitH(sf::FloatRect *pBoundsA, sf::FloatRect *pBoundsB, float height)
	{
		const float ch = pBoundsA->width - height;
		pBoundsA->width = height;
		*pBoundsB = *pBoundsA;
		pBoundsB->left += pBoundsA->width;
		pBoundsB->width = ch;
	}
	static inline void applyMargin(sf::FloatRect *pBounds, float margin)
	{
		pBounds->left += margin;
		pBounds->top += margin;
		pBounds->width -= margin;
		pBounds->height -= margin;
	}


protected:
	const void *m_pDrawControl;
	const void *m_pHotControl;
	float *m_pHotControlOffsetY;
	int *m_pHotControlSelectedItem;
	int m_HotControlType;
	sf::FloatRect m_HotControlBounds;

	sf::Vector2u m_ScreenSize;
	bool m_MouseLeftClicked;
	bool m_MouseRightClicked;
	sf::Vector2i m_MousePosition;
	const sf::Font &m_Font;

	char *m_pEditBuffer;
	int m_EditMaxLength;

private:
	bool isMouseInsideControl(sf::Shape *pControlShape) const noexcept;
	bool isMouseInsideControl(sf::Text *pControlShape) const noexcept;
	bool isMouseInsideArea(sf::FloatRect *pArea) const noexcept;

	static sf::FloatRect s_ListViewBounds;
	static int s_ListViewNumRows;
	static int s_ListViewCurrentRow;
	static int *s_pListViewSelectedRow;
	static float *s_pListViewScrollOffset;
	static float s_ListViewHeightPerRow;
	static sf::Vector2f s_ListViewContentSize;
};

#endif
