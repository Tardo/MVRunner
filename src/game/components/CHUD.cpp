/* (c) Alexandre Díaz. See licence.txt in the root of the distribution for more information. */

#include <engine/CGame.hpp>
#include <engine/CLocalization.hpp>
#include <game/version.h>
#include "CHUD.hpp"

CHUD::CHUD(CGameClient *pGameClient) noexcept
: CComponent(pGameClient)
{ }
CHUD::~CHUD() noexcept
{
	#ifdef DEBUG_DESTRUCTORS
	ups::msgDebug("CHUD", "Deleted");
	#endif
}

void CHUD::draw(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (Client()->getRenderMode() != RENDER_MODE_NORMAL)
		return;

	target.setView(Client()->getHudView());

	if (Client()->Menus().getActive() == CMenus::NONE)
	{
		renderMessages(target, states);
		renderTime(target, states);
	}
	renderCursor(target, states);
}

void CHUD::renderTime(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, target.getView());

	sf::Int64 endTime = Client()->isPlayerTimerRun()?ups::timeGet():Client()->m_TimerPlayerEnd;
	const sf::Int64 elapsedTimeMili = endTime - Client()->m_TimerPlayerStart;
	if (elapsedTimeMili > 0)
	{
		const int elapsedTimeSeconds = elapsedTimeMili / ups::timeFreq();
		char aBuff[10];
		sprintf(aBuff, "%02d:%02d.%03d", elapsedTimeSeconds/60, elapsedTimeSeconds%60, (int)(elapsedTimeMili%1000));
		Client()->UI().doLabel(target, states, aBuff, sf::FloatRect(0.0f, 0.0f, rectArea.width, 80.0f), sf::Color::Red, 42u, CUI::ALIGN_CENTER);
	}

//	// TEST LIST A
//	static int selectedRow = -1;
//	static float lvScrollOffset = 0.0f;
//	static int listRowIds[40];
//	char aBuff[16];
//	Client()->UI().doListInit(target, states, &selectedRow, sf::FloatRect(100.0f, 100.0f, 200.0f, 600.0f), 40, &selectedRow, 22.0f, &lvScrollOffset);
//	for (int i=0; i<40; ++i)
//	{
//		CListItem item = Client()->UI().doNextListItem(&selectedRow, &listRowIds[i]);
//		if (item.m_Visible)
//		{
//			snprintf(aBuff, sizeof(aBuff), "TEST #%d", i);
//			Client()->UI().doLabel(target, states, aBuff, item.m_Bounds, item.m_Selected?sf::Color::Yellow:sf::Color::Red, 18.0f, CUI::ALIGN_LEFT);
//			if (item.m_Selected)
//			{
//				//ups::msgDebug("LIST", "Select Row: %d", i);
//			}
//		}
//	}
//	Client()->UI().doListEnd(target, states, &selectedRow);
//	// END TEST LIST A
//
//	// TEST LIST B
//	static int selectedRowB = -1;
//	static float lvScrollOffsetB = 20.0f;
//	static int listRowIdsB[40];
//	char aBuffB[16];
//	Client()->UI().doListInit(target, states, &selectedRowB, sf::FloatRect(400.0f, 100.0f, 300.0f, 600.0f), 40, &selectedRowB, 22.0f, &lvScrollOffsetB);
//	for (int i=0; i<40; ++i)
//	{
//		CListItem item = Client()->UI().doNextListItem(&selectedRowB, &listRowIdsB[i]);
//		if (item.m_Visible)
//		{
//			sf::FloatRect RightColumn;
//			CUI::splitVMid(&item.m_Bounds, &RightColumn);
//			snprintf(aBuffB, sizeof(aBuffB), "TEST A #%d", i);
//			Client()->UI().doLabel(target, states, aBuffB, item.m_Bounds, item.m_Selected?sf::Color::Yellow:sf::Color::Red, 18.0f, CUI::ALIGN_LEFT);
//			snprintf(aBuffB, sizeof(aBuffB), "TEST B #%d", i);
//			Client()->UI().doLabel(target, states, aBuffB, RightColumn, item.m_Selected?sf::Color::Yellow:sf::Color::Red, 18.0f, CUI::ALIGN_LEFT);
//			if (item.m_Selected)
//			{
//				//ups::msgDebug("LIST", "Select Row: %d", i);
//			}
//		}
//	}
//	Client()->UI().doListEnd(target, states, &selectedRowB);
//	// END TEST LIST B
//
//	// CHECK TEST A
//	static bool selectedCheckA = false;
//	Client()->UI().doCheck(target, states, &selectedCheckA, sf::FloatRect(800.0f, 100.0f, 140.0f, 16.0f), &selectedCheckA, "Test Check A", 18, sf::Color::White, CUI::ALIGN_LEFT);
//
//	// CHECK TEST B
//	static bool selectedCheckB = false;
//	Client()->UI().doCheck(target, states, &selectedCheckB, sf::FloatRect(800.0f, 120.0f, 140.0f, 16.0f), &selectedCheckB, "Test Check B", 18, sf::Color::Yellow, CUI::ALIGN_LEFT);
//
//	// CHECK EDIT
//	static char editStringA[20] = {0};
//	Client()->UI().doEdit(target, states, editStringA, sf::FloatRect(800.0f, 160.0f, 140.0f, 20.0f), editStringA, 18, sf::Color::Yellow, 20);

}

void CHUD::renderMessages(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	if (Client()->Menus().getActive() != CMenus::NONE)
		return;

	sf::FloatRect rectArea;
	Client()->getViewportGlobalBounds(&rectArea, Client()->getHudView());

	sf::Text sfStr;
	sfStr.setFont(Client()->Assets().getDefaultFont());

	char aBuff[128];
	sfStr.setCharacterSize(24);
	snprintf(aBuff, sizeof(aBuff), "FPS: %u [Min: %u] [%.2fms]", Client()->m_FPS, Client()->m_MinFPS, Client()->getDeltaTime()*1000.0f);
	sfStr.setString(aBuff);
	sfStr.setPosition(rectArea.width-sfStr.getLocalBounds().width-10.0f, 0.0f);
	sfStr.setFillColor(sf::Color::Red);
	target.draw(sfStr, states);

	if (ups::timeGet() - Client()->m_TimerBroadcast < ups::timeFreq()*Client()->m_BroadcastDuration)
	{
		sfStr.setCharacterSize(128);
		sfStr.setString(Client()->m_aBroadcastMsg);
		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f+2.0f, 2.0f);
		sfStr.setFillColor(sf::Color::Yellow);
		target.draw(sfStr, states);

		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f, 0.0f);
		sfStr.setFillColor(sf::Color::Red);
		target.draw(sfStr, states);
	}

	if (Client()->m_aHelpMsg[0] != 0)
	{
		sfStr.setCharacterSize(92);
		sfStr.setString(Client()->m_aHelpMsg);
		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f+2.0f, 78.0f+2.0f);
		sfStr.setFillColor(sf::Color::Black);
		target.draw(sfStr, states);

		sfStr.setPosition(rectArea.width/2.0f-sfStr.getLocalBounds().width/2.0f, 78.0f);
		sfStr.setFillColor(sf::Color::White);
		target.draw(sfStr, states);
	}
}

void CHUD::renderCursor(sf::RenderTarget& target, sf::RenderStates states) const noexcept
{
	sf::Vertex line[] =
	{
		sf::Vertex(sf::Vector2f(Client()->UI().getMousePos().x-10.0f, Client()->UI().getMousePos().y), sf::Color::White),
		sf::Vertex(sf::Vector2f(Client()->UI().getMousePos().x+10.0f, Client()->UI().getMousePos().y), sf::Color::White),
		sf::Vertex(sf::Vector2f(Client()->UI().getMousePos().x, Client()->UI().getMousePos().y-10.0f), sf::Color::White),
		sf::Vertex(sf::Vector2f(Client()->UI().getMousePos().x, Client()->UI().getMousePos().y+10.0f), sf::Color::White)
	};

	target.draw(line, 4, sf::Lines);
}
