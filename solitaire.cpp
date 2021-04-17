

/*
This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "wx/wx.h"
#include "cards/cards.h"
#include "solitaire.xpm"
#include <map>
#include <string>

#include <algorithm>    // std::shuffle
#include <array>        // std::array
#include <random>       // std::default_random_engine
#include <chrono>       // std::chrono::system_clock


#define WIDTH 69 //card width, from .xpm images
#define HEIGHT 100 //card height
#define STAGGER 20 //for staggered decks, the vertical offset of each card in the stack
#define PILEOFFSET 3 //used when creating piles under tableau, so they are offset slightly to indicate there are cards in the pile

#define NEWGAME1 1000
#define NEWGAME3 1001

/* Program Organization

After the obligatory wxwidgets wxApp and wxFrame, there are three application classes:

	- Card: Encapsulates all the attributes of a single card. Created using the card definitions in cards/cards.h
	- Stack: Implements the stack used to define all the places in the game where cards can exist. Defines behaviors
		that are used to move cards between stacks in a way to account for all cards.
	- CardPane: A subclass of wxPane where all the application behaviors, including stack transfer rules, are implemented.

*/

class Card
{
public:
	Card(const char * image[], std::string cardsuit, std::string cardrank, std::string cardcolor, int ranknum, int posx, int posy)
	{
		face = wxBitmap(image);
		b.x = posx;
		b.y = posy;
		b.width = WIDTH;
		b.height = HEIGHT;
		suit = cardsuit;
		rankstring = cardrank;
		color = cardcolor;
		rank = ranknum;
	}
	
	Card(const char * image[], std::string cardsuit, std::string cardrank, std::string cardcolor, int ranknum, wxPoint pos)
	{
		face = wxBitmap(image);
		b.x = pos.x;
		b.y = pos.y;
		b.width = WIDTH;
		b.height = HEIGHT;
		suit = cardsuit;
		rankstring = cardrank;
		color = cardcolor;
		rank = ranknum;
	}
	
	Card() { 
		face = wxBitmap(0,0);
		b.x = 0;
		b.y = 0;
		b.width = WIDTH;
		b.height = HEIGHT;
		suit = "(blank)";
		rankstring = "(blank)";
		color = "(blank)";
		rank = 0;
	}
	
	int GetRank()
	{
		return rank;
	}
	
	std::string GetRankString()
	{
		return rankstring;
	}
	
	std::string GetSuit()
	{
		return suit;
	}
	
	std::string GetColor()
	{
		return color;
	}
	
	bool HitTest(int locx, int locy)
	{
		return b.Contains(locx, locy);
	}
	
	wxPoint GetPosition()
	{
		wxPoint p;
		p.x = b.x;
		p.y = b.y;
		return p;
	}
	
	void SetPosition(int locx, int locy)
	{
		b.x = locx;
		b.y = locy;
	}
	
	void SetPosition(wxPoint pos)
	{
		b.x = pos.x;
		b.y = pos.y;
	}
	
	void MovePosition(int dx, int dy)
	{
		b.x += dx;
		b.y += dy;
	}
	
	int GetX()
	{
		return b.x;
	}
	
	int GetY()
	{
		return b.y;
	}
	
	void DrawCard(wxDC& dc, bool faceup=true)
	{
		if (faceup) {
			if (face.IsOk()) dc.DrawBitmap(face,b.x,b.y);
		}
		else {
			dc.SetPen(*wxLIGHT_GREY_PEN);
			dc.DrawRoundedRectangle (b.x,  b.y,  b.width,  b.height,  3);
		}
	}
	
	wxRect& GetRect()
	{
		return b;
	}

	//animate velocities;
	int vert, horiz;

private:
	wxBitmap face;
	wxRect b;
	std::string suit, rankstring, color;
	int rank;
};


class Stack
{
public:

	Stack() 
	{ 
		bounds.width = WIDTH;
		bounds.height = HEIGHT;
		facup = true;
		outln = true;
		stag = false;
	}

	Stack(int x, int y, int w, int h, bool faceup=true, bool outline=true, bool staggered=false)
	{
		bounds.x = xorig = x;
		bounds.y = yorig = y;
		bounds.width = worig = w;
		bounds.height = horig = h;
		facup = faceup;
		outln = outline;
		stag = staggered;
	}
	
	std::string GetName()
	{
		return name;
	}
	
	int GetNumberOfCards()
	{
		return (int) deck.size();
	}
	
	wxRect GetBounds()
	{
		return bounds;
	}
	
	void SetName(std::string stackname)
	{
		name = stackname;
	}
	
	void SetOutline(bool outline)
	{
		outln = outline;
	}
	
	void SetFaceup(bool faceup)
	{
		facup = faceup;
	}
	
	void SetStaggered(bool staggered)
	{
		stag = staggered;
	}
	
	void ClearCards()
	{
		deck.clear();
		RecomputeBounds();
	}
	
	std::vector<Card> GetCards()
	{
		return deck;
	}
	
	Card& TopCard()
	{
		return deck.back();
	}
	
	Card& BottomCard()
	{
		return deck.front();
	}
	
	void AddCard(Card c)
	{
		wxPoint pos;
		if (deck.size()<=1) {
			pos = GetPosition();
		}
		else {
			pos = deck.back().GetPosition();
			if (stag) pos.y += STAGGER;
		}
		c.SetPosition(pos);
		deck.push_back(c);
		RecomputeBounds();
	}
	
	void AddCards(std::vector<Card> cards)
	{
		if (cards.size() == 0) return;
		for (std::vector<Card>::iterator it=cards.begin(); it!=cards.end(); ++it) {
			wxPoint pos;
			if (deck.size() == 0) {
				pos = GetPosition();
			}
			else {
				pos = deck.back().GetPosition();
				if (stag) pos.y += STAGGER;
			}
			(*it).SetPosition(pos);
			deck.push_back(*it);
		}
		RecomputeBounds();
	}
	
	std::vector<Card> ExtractCards(int index)
	{
		std::vector<Card> stack;
		while (index < deck.size()) {
			stack.push_back(deck[index]);
			std::vector<Card>::iterator it = deck.begin() + index;
			deck.erase(it);
		}
		RecomputeBounds();
		return stack;
	}
	
	Card ExtractTopCard()
	{
		Card c = deck.back();
		deck.pop_back();
		RecomputeBounds();
		return c;
	}

	void SetPosition(int locx, int locy)
	{
		bounds.x = xorig = locx;
		bounds.y = yorig = locy;
		for (std::vector<Card>::iterator it = deck.begin(); it !=deck.end(); ++it) {
			if (stag) 
				it->SetPosition(locx, locy+((it-deck.begin())*STAGGER));
			else
				it->SetPosition(locx, locy);
		}
		RecomputeBounds();
	}

	void SetPosition(wxPoint pos)
	{
		bounds.x = xorig = pos.x;
		bounds.y = yorig = pos.y;
		for (std::vector<Card>::iterator it = deck.begin(); it !=deck.end(); ++it) {
			if (stag) pos.y+=STAGGER;
			it->SetPosition(pos);
		}
		RecomputeBounds();
	}
	
	void MovePosition(int dx, int dy)
	{
		bounds.x += dx;
		bounds.y += dy;
		xorig += dx;
		xorig += dy;
		for (std::vector<Card>::iterator it = deck.begin(); it !=deck.end(); ++it) {
			if (stag) 
				it->SetPosition(bounds.x, bounds.y+((it-deck.begin())*STAGGER));
			else
				it->SetPosition(bounds.x, bounds.y);
		}
		RecomputeBounds();
	}
	
	wxPoint GetPosition()
	{
		return wxPoint(bounds.x, bounds.y);
	}
	
	wxRect GetBoundingBox()
	{
		return bounds;
	}
	
	wxPoint GetCardPosition(int index)
	{
		if (deck.size() == 0) {
			wxPoint p;
			p.x = -1;
			p.y = -1;
			return p;
		}
		return deck[index].GetPosition();
	}
	
	bool HitTest(int locx, int locy)
	{
		wxPoint loc;
		loc.x = locx; loc.y = locy;
		return bounds.Contains(loc);
	}
	
	bool HitTest(wxPoint loc)
	{
		return bounds.Contains(loc);
	}

	int HitTestCard(int locx, int locy)
	{
		if (bounds.Contains(locx, locy) & deck.size() > 0) {
			for (unsigned i=deck.size()-1; i>=0; i--) {
				if (deck[i].HitTest(locx, locy)) {
					return i;
				}
			}
		}
		return -1;
	}
	
	void DrawCards(wxDC& dc)
	{
		if (outln) {
			dc.SetPen(*wxLIGHT_GREY_PEN);
			dc.DrawRoundedRectangle (bounds.x,  bounds.y,  WIDTH,  HEIGHT,  3);
		}
		for (std::vector<Card>::iterator it = deck.begin(); it !=deck.end(); ++it)
			it->DrawCard(dc, facup);
	}

private:
	void RecomputeBounds()
	{
		if (deck.size() == 0) {
			//bounds.x = xorig;
			//bounds.y = yorig;
			bounds.width = WIDTH;
			bounds.height = HEIGHT;
		}
		else {
			for (std::vector<Card>::iterator it = deck.begin(); it !=deck.end(); ++it)
				bounds.Union((*it).GetRect());
		}
	}
	
	std::vector<Card> deck;
	std::string name;
	wxRect bounds;  //contains the x, y, width, height
	bool facup, outln, stag;
	int xorig, yorig, worig, horig;
};

class myButton
{
public:
	myButton() { }

	myButton(wxWindow* parent, wxString label, wxPoint p)
	{
		prnt = parent;
		lbl = label;
		bnds.x = p.x;
		bnds.y = p.y;
		wxClientDC dc(parent);
		wxSize s = dc.GetTextExtent(lbl);
		bnds.width = s.GetWidth()+10;
		bnds.height = s.GetHeight()+10;
		c = false;
	}
	
	void SetClicked(bool clicked)
	{
		c = clicked;
	}
	
	wxRect GetBounds()
	{
		return bnds;
	}
	
	wxSize GetSize()
	{
		wxSize s(bnds.width, bnds.height);
		return s;
	}
	
	bool HitTest(wxPoint p)
	{
		return bnds.Contains(p);
	}
	
	bool HitTest(int x, int y)
	{
		return bnds.Contains(x, y);
	}
	
	void render(wxDC& dc)
	{
		wxPen p = dc.GetPen();
		if (c) dc.SetPen(wxPen(wxColour(0,0,0), 3));
		dc.DrawRoundedRectangle(bnds,3);
		dc.SetPen(p);
		dc.DrawText(lbl, bnds.x+5, bnds.y+5);
	}

private:
	wxWindow* prnt;
	wxRect bnds;
	wxString lbl;
	bool c;
};


class CardPane: wxPanel
{
public:
	CardPane(wxWindow *parent, wxWindowID id) :wxPanel(parent, id)
	{
		originstack = NULL;
		//SetBackgroundColour(wxColour(34, 139, 34)); //forest green
		SetBackgroundColour(wxColour(255, 255, 255));
		SetDoubleBuffered(true);
		
		//for each stack in the game:
		deck.SetName("deck"); deck.SetPosition(50,100); deck.SetFaceup(false); //deck.SetCapacity(52);
		play.SetName("play"); play.SetPosition(150,100); 
		suit1.SetName("suit1"); suit1.SetPosition(350,100); 
		suit2.SetName("suit2"); suit2.SetPosition(450,100); 
		suit3.SetName("suit3"); suit3.SetPosition(550,100); 
		suit4.SetName("suit4"); suit4.SetPosition(650,100); 
		pile1.SetName("pile1"); pile1.SetPosition(50-PILEOFFSET,250-PILEOFFSET); pile1.SetFaceup(false); 
		pile2.SetName("pile2"); pile2.SetPosition(150-PILEOFFSET,250-PILEOFFSET); pile2.SetFaceup(false); 
		pile3.SetName("pile3"); pile3.SetPosition(250-PILEOFFSET,250-PILEOFFSET); pile3.SetFaceup(false); 
		pile4.SetName("pile4"); pile4.SetPosition(350-PILEOFFSET,250-PILEOFFSET); pile4.SetFaceup(false); 
		pile5.SetName("pile5"); pile5.SetPosition(450-PILEOFFSET,250-PILEOFFSET); pile5.SetFaceup(false); 
		pile6.SetName("pile6"); pile6.SetPosition(550-PILEOFFSET,250-PILEOFFSET); pile6.SetFaceup(false); 
		pile7.SetName("pile7"); pile7.SetPosition(650-PILEOFFSET,250-PILEOFFSET); pile7.SetFaceup(false); 
		tableau1.SetName("tableau1"); tableau1.SetPosition(50,250); tableau1.SetStaggered(true); 
		tableau2.SetName("tableau2"); tableau2.SetPosition(150,250); tableau2.SetStaggered(true); 
		tableau3.SetName("tableau3"); tableau3.SetPosition(250,250); tableau3.SetStaggered(true); 
		tableau4.SetName("tableau4"); tableau4.SetPosition(350,250); tableau4.SetStaggered(true); 
		tableau5.SetName("tableau5"); tableau5.SetPosition(450,250); tableau5.SetStaggered(true); 
		tableau6.SetName("tableau6"); tableau6.SetPosition(550,250); tableau6.SetStaggered(true); 
		tableau7.SetName("tableau7"); tableau7.SetPosition(650,250); tableau7.SetStaggered(true); 
		
		movestack.SetPosition(0,0); movestack.SetOutline(false); movestack.SetStaggered(true); 
		moving = false;
		nullstack.SetName("nullstack");
		
		animate = false;
		nextcard = 0;
		win = false;  //used to keep the deal following a win from counting as a loss.
		
		newgame1 = myButton(this, "New Game (Draw 1)", wxPoint(1,1));
		wxSize s1 = newgame1.GetSize();
		newgame3 = myButton(this, "New Game (Draw 3)", wxPoint(s1.GetWidth()+2,1));
		
		wins = 0;
		losses = 0;
		
		Bind(wxEVT_PAINT, &CardPane::OnPaint, this);
		Bind(wxEVT_LEFT_DCLICK, &CardPane::OnLeftDoubleClick, this);
		Bind(wxEVT_LEFT_DOWN, &CardPane::OnLeftDown, this);
		Bind(wxEVT_MOTION, &CardPane::OnMotion, this);
		Bind(wxEVT_LEFT_UP, &CardPane::OnLeftUp, this);

		t.SetOwner(this);
		Bind(wxEVT_TIMER, &CardPane::OnTimer, this);
	}

	void OnPaint(wxPaintEvent& WXUNUSED(event))
	{
		wxPaintDC dc(this);
		render(dc);
	}
	
	void render(wxDC& dc)
	{
		
		if (deck.GetNumberOfCards() > 0) deck.DrawCards(dc);
		if (play.GetNumberOfCards() > 0) play.DrawCards(dc);
		suit1.DrawCards(dc);
		suit2.DrawCards(dc);
		suit3.DrawCards(dc);
		suit4.DrawCards(dc);
		if (pile1.GetNumberOfCards() > 0) pile1.DrawCards(dc);
		if (pile2.GetNumberOfCards() > 0) pile2.DrawCards(dc);
		if (pile3.GetNumberOfCards() > 0) pile3.DrawCards(dc);
		if (pile4.GetNumberOfCards() > 0) pile4.DrawCards(dc);
		if (pile5.GetNumberOfCards() > 0) pile5.DrawCards(dc);
		if (pile6.GetNumberOfCards() > 0) pile6.DrawCards(dc);
		if (pile7.GetNumberOfCards() > 0) pile7.DrawCards(dc);
		if (tableau1.GetNumberOfCards() > 0) tableau1.DrawCards(dc);
		if (tableau2.GetNumberOfCards() > 0) tableau2.DrawCards(dc);
		if (tableau3.GetNumberOfCards() > 0) tableau3.DrawCards(dc);
		if (tableau4.GetNumberOfCards() > 0) tableau4.DrawCards(dc);
		if (tableau5.GetNumberOfCards() > 0) tableau5.DrawCards(dc);
		if (tableau6.GetNumberOfCards() > 0) tableau6.DrawCards(dc);
		if (tableau7.GetNumberOfCards() > 0) tableau7.DrawCards(dc);
		
		movestack.DrawCards(dc);
		
		if (animate) animatecard.DrawCard(dc);
		
		newgame1.render(dc);
		newgame3.render(dc);
		
		wxSize s = GetSize();
		
		dc.DrawText(wxString::Format("Draw-%d",numberofcardstodraw), 500,10);
		if (wins == 1)
			dc.DrawText(wxString::Format("%d Win",wins),  500,30);
		else
			dc.DrawText(wxString::Format("%d Wins",wins),  500,30);
		if (losses == 1)
			dc.DrawText(wxString::Format("%d Loss",losses), 500,50);
		else
			dc.DrawText(wxString::Format("%d Losses",losses), 500,50);
	}

	
	//compute the area of a rectangle
	int rectArea(wxRect r)
	{
		return r.width * r.height;
	}
	
	Stack& HitTest(wxRect r)
	{
		//max hit area:
		int hitarea = 0;
		
		//1. Store the hit area of the intersection of each static stack with the movestack: 
		int deckarea = rectArea(deck.GetBounds().Intersect(r));
		int playarea = rectArea(play.GetBounds().Intersect(r));
		int suit1area = rectArea(suit1.GetBounds().Intersect(r));
		int suit2area = rectArea(suit2.GetBounds().Intersect(r));
		int suit3area = rectArea(suit3.GetBounds().Intersect(r));
		int suit4area = rectArea(suit4.GetBounds().Intersect(r));
		int tableau1area = rectArea(tableau1.GetBounds().Intersect(r));
		int tableau2area = rectArea(tableau2.GetBounds().Intersect(r));
		int tableau3area = rectArea(tableau3.GetBounds().Intersect(r));
		int tableau4area = rectArea(tableau4.GetBounds().Intersect(r));
		int tableau5area = rectArea(tableau5.GetBounds().Intersect(r));
		int tableau6area = rectArea(tableau6.GetBounds().Intersect(r));
		int tableau7area = rectArea(tableau7.GetBounds().Intersect(r));
		
		//2. Find the max of the hit areas:
		if (deckarea > hitarea) hitarea = deckarea;
		if (playarea > hitarea) hitarea = playarea;
		if (suit1area > hitarea) hitarea = suit1area;
		if (suit2area > hitarea) hitarea = suit2area;
		if (suit3area > hitarea) hitarea = suit3area;
		if (suit4area > hitarea) hitarea = suit4area;
		if (tableau1area > hitarea) hitarea = tableau1area;
		if (tableau2area > hitarea) hitarea = tableau2area;
		if (tableau3area > hitarea) hitarea = tableau3area;
		if (tableau4area > hitarea) hitarea = tableau4area;
		if (tableau5area > hitarea) hitarea = tableau5area;
		if (tableau6area > hitarea) hitarea = tableau6area;
		if (tableau7area > hitarea) hitarea = tableau7area;
		
		//3. If a static stack's hitarea is greater than the max hit area, return that stack reference:
		if (deckarea == hitarea) return deck;
		if (playarea == hitarea) return play;
		if (suit1area == hitarea) return suit1;
		if (suit2area == hitarea) return suit2;
		if (suit3area == hitarea) return suit3;
		if (suit4area == hitarea) return suit4;
		if (tableau1area == hitarea) return tableau1;
		if (tableau2area == hitarea) return tableau2;
		if (tableau3area == hitarea) return tableau3;
		if (tableau4area == hitarea) return tableau4;
		if (tableau5area == hitarea) return tableau5;
		if (tableau6area == hitarea) return tableau6;
		if (tableau7area == hitarea) return tableau7;
		return nullstack;
	}
	
	
	Stack& HitTest(int x, int y)
	{
		if (deck.HitTest(x,y)) return deck;
		if (play.HitTest(x,y)) return play;
		if (suit1.HitTest(x,y)) return suit1;
		if (suit2.HitTest(x,y)) return suit2;
		if (suit3.HitTest(x,y)) return suit3;
		if (suit4.HitTest(x,y)) return suit4;
		if (tableau1.HitTest(x,y)) return tableau1;
		if (tableau2.HitTest(x,y)) return tableau2;
		if (tableau3.HitTest(x,y)) return tableau3;
		if (tableau4.HitTest(x,y)) return tableau4;
		if (tableau5.HitTest(x,y)) return tableau5;
		if (tableau6.HitTest(x,y)) return tableau6;
		if (tableau7.HitTest(x,y)) return tableau7;
		return nullstack;
	}
	
	void movestackSetup(Stack& stack, int x, int y, int cardindex)
	{
		movestack.SetPosition(stack.GetCardPosition(cardindex));
		movestack.AddCards(stack.ExtractCards(cardindex));
		originstack = &stack;
		moving = true;
		prevx = x;
		prevy = y;
		Refresh();
	}

	void OnLeftDown(wxMouseEvent& event)
	{
		//printf("Left Down event...\n"); fflush(stdout);
		int cardindex;
		int x = event.GetX();
		int y = event.GetY();

		animate = false;
		((wxFrame *) GetParent())->SetStatusText("");
		
		if (newgame1.HitTest(x,y)) { newgame1.SetClicked(true); Refresh(); return; }
		if (newgame3.HitTest(x,y)) { newgame3.SetClicked(true); Refresh(); return; }
		
		//for each stack in the game:
		if (deck.HitTest(x,y)) {
			movestack.SetPosition(deck.GetPosition());
			originstackname = deck.GetName();
			originstack = &deck;
			moving = true;
			prevx = x;
			prevy = y;
			//printf("OnLeftDown: movestack: pos=%d,%d\n", movestack.GetPosition().x, movestack.GetPosition().y); fflush(stdout);
			Refresh();
		}
		else if (play.HitTest(x,y))
			originstackname = play.GetName();
			movestack.SetPosition(play.GetPosition());
			if ((cardindex = play.HitTestCard(x,y)) > -1 ) 
				movestackSetup(play, x, y, cardindex);
		else if (suit1.HitTest(x,y)) {
			originstackname = suit1.GetName();
			movestack.SetPosition(suit1.GetPosition());
			if ((cardindex = suit1.HitTestCard(x,y)) > -1 ) 
				movestackSetup(suit1, x, y, cardindex);
		}
		else if (suit2.HitTest(x,y)) {
			originstackname = suit2.GetName();
			movestack.SetPosition(suit2.GetPosition());
			if ((cardindex = suit2.HitTestCard(x,y)) > -1 ) 
				movestackSetup(suit2, x, y, cardindex);
		}
		else if (suit3.HitTest(x,y)) {
			originstackname = suit3.GetName();
			movestack.SetPosition(suit2.GetPosition());
			if ((cardindex = suit3.HitTestCard(x,y)) > -1 ) 
				movestackSetup(suit3, x, y, cardindex);
		}
		else if (suit4.HitTest(x,y)) {
			originstackname = suit4.GetName();
			movestack.SetPosition(suit2.GetPosition());
			if ((cardindex = suit4.HitTestCard(x,y)) > -1 ) 
				movestackSetup(suit4, x, y, cardindex);
		}
		else if (tableau1.HitTest(x,y)) {
			originstackname = tableau1.GetName();
			movestack.SetPosition(tableau1.GetPosition());
			if ((cardindex = tableau1.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau1, x, y, cardindex);
		}
		else if (tableau2.HitTest(x,y)) {
			originstackname = tableau2.GetName();
			movestack.SetPosition(tableau2.GetPosition());
			if ((cardindex = tableau2.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau2, x, y, cardindex);
		}
		else if (tableau3.HitTest(x,y)) {
			originstackname = tableau3.GetName();
			movestack.SetPosition(tableau3.GetPosition());
			if ((cardindex = tableau3.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau3, x, y, cardindex);
		}
		else if (tableau4.HitTest(x,y)) {
			originstackname = tableau4.GetName();
			movestack.SetPosition(tableau4.GetPosition());
			if ((cardindex = tableau4.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau4, x, y, cardindex);
		}
		else if (tableau5.HitTest(x,y)) {
			originstackname = tableau5.GetName();
			movestack.SetPosition(tableau5.GetPosition());
			if ((cardindex = tableau5.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau5, x, y, cardindex);
		}
		else if (tableau6.HitTest(x,y)) {
			originstackname = tableau6.GetName();
			movestack.SetPosition(tableau6.GetPosition());
			if ((cardindex = tableau6.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau6, x, y, cardindex);
		}
		else if (tableau7.HitTest(x,y)) {
			originstackname = tableau7.GetName();
			movestack.SetPosition(tableau7.GetPosition());
			if ((cardindex = tableau7.HitTestCard(x,y)) > -1 ) 
				movestackSetup(tableau7, x, y, cardindex);
		}
	}
	
	void OnLeftDoubleClick(wxMouseEvent& event)
	{
		//printf("DoubleClick event...\n"); fflush(stdout);
		int x = event.GetX();
		int y = event.GetY();
		
		if (HitTest(x,y).GetName() == "nullstack") return;
		if (HitTest(x,y).GetName() == "deck") return;
		
		DoubleClickRules(HitTest(x,y));
	}
	
	void OnMotion(wxMouseEvent& event)
	{
		//printf("Motion event...\n"); fflush(stdout);
		wxPoint n;
		if (moving) {
			int x = event.GetX();
			int y = event.GetY();
			n.x = x - prevx;
			n.y = y - prevy;
			movestack.MovePosition(n.x, n.y);
			//printf("OnMotion: movestack: pos=%d,%d\n", movestack.GetPosition().x, movestack.GetPosition().y); fflush(stdout);
			prevx = x;
			prevy = y;
			Refresh();
		}
	}
	
	
	void TableauLandingRules(Stack& tableau, Stack& pile)
	{
		if (originstackname == tableau.GetName() & tableau.GetNumberOfCards() == 0) { //mouse click on tableau
			if (movestack.GetNumberOfCards() > 0) {
				tableau.AddCards(movestack.GetCards());  //put 'em back in the tableau
			}	
			else {
				if (pile.GetNumberOfCards() > 0) tableau.AddCard(pile.ExtractTopCard()); //extract a new top card from the pile
			}
		}
		else { //a drag from somewhere else
			if (tableau.GetNumberOfCards() == 0 & //empty tableau and pile, 
				pile.GetNumberOfCards() == 0 &
				movestack.GetNumberOfCards() >= 1) { 
				if (movestack.BottomCard().GetRankString() == "king") //only allow a king
				{
					tableau.AddCards(movestack.GetCards());
					return;
				}
				else {
					ReturnStack(); 
					return;
				}
			}
			if (tableau.GetNumberOfCards() > 0 & 
				tableau.TopCard().GetRank() == movestack.BottomCard().GetRank() + 1 & 
				tableau.TopCard().GetColor() != movestack.BottomCard().GetColor()) 
			{
					tableau.AddCards(movestack.GetCards());
					return;
			}
			else
			{
				ReturnStack();
			}
		}
	}
	
	void SuitLandingRules(Stack& suit)
	{
		if (originstackname == suit.GetName() & suit.GetNumberOfCards() == 0) { //mouse click on tableau
			if (movestack.GetNumberOfCards() > 0) {
				suit.AddCards(movestack.GetCards());  //put 'em back in the tableau
			}	
			return;
		}
		if (suit.GetNumberOfCards() == 0 & 
			movestack.GetNumberOfCards() == 1 & 
			movestack.BottomCard().GetRankString() == "ace") 
		{
			suit.AddCards(movestack.GetCards());
			return;
		}
		if (suit.GetNumberOfCards() > 0 &movestack.GetNumberOfCards() == 1) 
		{
			if (movestack.BottomCard().GetSuit() == suit.TopCard().GetSuit() &
				movestack.BottomCard().GetRank()-1 == suit.TopCard().GetRank())
			{
				suit.AddCards(movestack.GetCards());
				return;
			}
		}
		ReturnStack();
	}
	
	void ReturnStack() //puts the movestack back on the origin stack
	{
		originstack->AddCards(movestack.GetCards()); 
	}
	
	void OnLeftUp(wxMouseEvent& event)
	{
		//printf("Left Up event...\n"); fflush(stdout);
		int x = event.GetX();
		int y = event.GetY();
		
		if (!moving) {
			if (newgame1.HitTest(x,y)) { OnNewGame(1); newgame1.SetClicked(false); Refresh(); return; }
			if (newgame3.HitTest(x,y)) { OnNewGame(3); newgame3.SetClicked(false); Refresh(); return; }
		}
		
		//printf("Hit stack: %s Origin stack: %s\n", HitTest(movestack.GetBounds()).GetName().c_str(), originstackname.c_str()); fflush(stdout);
		
		std::string hitstack = HitTest(movestack.GetBounds()).GetName();
		
		//for each stack in the game:
		if (deck.HitTest(x,y)) {
			if (originstackname == deck.GetName()) { //deck is clicked
				if (deck.GetNumberOfCards() > 0) {
					for (unsigned i=1; i<=numberofcardstodraw; i++) {
						if (deck.GetNumberOfCards() > 0) play.AddCard(deck.ExtractTopCard());
					}
				}
				else {
					while (play.GetNumberOfCards() > 0) 
						deck.AddCard(play.ExtractTopCard());
					play.ClearCards();
				}
			}
			else ReturnStack();
		}
		else if (hitstack == "play") {
			originstack->AddCards(movestack.GetCards()); //put 'em back where they came from...
		}
		
		else if (hitstack == "suit1") {
			SuitLandingRules(suit1);
		}
		else if (hitstack == "suit2") {
			SuitLandingRules(suit2);
		}
		else if (hitstack == "suit3") {
			SuitLandingRules(suit3);
		}
		else if (hitstack == "suit4") {
			SuitLandingRules(suit4);
		}
			
		else if (hitstack == "tableau1") {
			TableauLandingRules(tableau1, pile1);
		}
		else if (hitstack == "tableau2") {
			TableauLandingRules(tableau2, pile2);
		}
		else if (hitstack == "tableau3") {
			TableauLandingRules(tableau3, pile3);
		}
		else if (hitstack == "tableau4") {
			TableauLandingRules(tableau4, pile4);
		}
		else if (hitstack == "tableau5") {
			TableauLandingRules(tableau5, pile5);
		}
		else if (hitstack == "tableau6") {
			TableauLandingRules(tableau6, pile6);
		}
		else if (hitstack == "tableau7") {
			TableauLandingRules(tableau7, pile7);
		}
		
		else 
			ReturnStack(); //put them back where they came from...
		
		movestack.ClearCards();
		
		Refresh();
			
		moving =  false;
		CheckWin();
		Refresh();
	}
	

	void DoubleClickRules(Stack& stack)
	{
		Card& stackcard = stack.TopCard();
		
		if (stackcard.GetRankString() == "ace") {
			if (suit1.GetNumberOfCards() == 0) suit1.AddCard(stack.ExtractTopCard());
			else if (suit2.GetNumberOfCards() == 0) suit2.AddCard(stack.ExtractTopCard());
			else if (suit3.GetNumberOfCards() == 0) suit3.AddCard(stack.ExtractTopCard());
			else if (suit4.GetNumberOfCards() == 0) suit4.AddCard(stack.ExtractTopCard());
			return;
		}
		
		if (suit1.GetNumberOfCards() > 0)
			if (stackcard.GetSuit() == suit1.TopCard().GetSuit() & stackcard.GetRank()-1 == suit1.TopCard().GetRank()) 
				suit1.AddCard(stack.ExtractTopCard());
		else if (suit2.GetNumberOfCards() > 0)
			if (stackcard.GetSuit() == suit2.TopCard().GetSuit() & stackcard.GetRank()-1 == suit2.TopCard().GetRank()) 
				suit2.AddCard(stack.ExtractTopCard());
		else if (suit3.GetNumberOfCards() > 0)
			if (stackcard.GetSuit() == suit3.TopCard().GetSuit() & stackcard.GetRank()-1 == suit3.TopCard().GetRank()) 
				suit3.AddCard(stack.ExtractTopCard());
		else if (suit4.GetNumberOfCards() > 0)
			if (stackcard.GetSuit() == suit4.TopCard().GetSuit() & stackcard.GetRank()-1 == suit4.TopCard().GetRank()) 
				suit4.AddCard(stack.ExtractTopCard());
		
		//originstackname = std::string();
	}

	void OnNewGame(int draw)
	{
		if (draw == 1) {
			NewGame(1);
			((wxFrame *) GetParent())->SetStatusText("New 1-card-draw game dealt.");
		}
		else if (draw == 3) {
			NewGame(3);
			((wxFrame *) GetParent())->SetStatusText("New 3-card-draw game dealt.");
		}
	}

	void NewGame(int numcarddraw)
	{
		ClearGame();
		ShuffleDeck();
		
		if (numberofcardstodraw != numcarddraw) {
			wins = 0;
			losses = 0;
		}
		else {
			if (!win) losses++;
		}
		win = false;
		
		numberofcardstodraw = numcarddraw;
		animate = false;
		
		//DeckDebug();
		
		tableau1.AddCard(deck.ExtractTopCard());
		pile2.AddCard(deck.ExtractTopCard());
		pile3.AddCard(deck.ExtractTopCard());
		pile4.AddCard(deck.ExtractTopCard());
		pile5.AddCard(deck.ExtractTopCard());
		pile6.AddCard(deck.ExtractTopCard());
		pile7.AddCard(deck.ExtractTopCard());
		
		tableau2.AddCard(deck.ExtractTopCard());
		pile3.AddCard(deck.ExtractTopCard());
		pile4.AddCard(deck.ExtractTopCard());
		pile5.AddCard(deck.ExtractTopCard());
		pile6.AddCard(deck.ExtractTopCard());
		pile7.AddCard(deck.ExtractTopCard());
		
		tableau3.AddCard(deck.ExtractTopCard());
		pile4.AddCard(deck.ExtractTopCard());
		pile5.AddCard(deck.ExtractTopCard());
		pile6.AddCard(deck.ExtractTopCard());
		pile7.AddCard(deck.ExtractTopCard());
		
		tableau4.AddCard(deck.ExtractTopCard());
		pile5.AddCard(deck.ExtractTopCard());
		pile6.AddCard(deck.ExtractTopCard());
		pile7.AddCard(deck.ExtractTopCard());
		
		tableau5.AddCard(deck.ExtractTopCard());
		pile6.AddCard(deck.ExtractTopCard());
		pile7.AddCard(deck.ExtractTopCard());
		
		tableau6.AddCard(deck.ExtractTopCard());
		pile7.AddCard(deck.ExtractTopCard());
		
		tableau7.AddCard(deck.ExtractTopCard());
		
		//StackDebug();
		
	}
	
	void ClearGame()
	{
		deck.ClearCards();
		play.ClearCards();
		suit1.ClearCards();
		suit2.ClearCards();
		suit3.ClearCards();
		suit4.ClearCards();
		pile1.ClearCards();
		pile2.ClearCards();
		pile3.ClearCards();
		pile4.ClearCards();
		pile5.ClearCards();
		pile6.ClearCards();
		pile7.ClearCards();
		tableau1.ClearCards();
		tableau2.ClearCards();
		tableau3.ClearCards();
		tableau4.ClearCards();
		tableau5.ClearCards();
		tableau6.ClearCards();
		tableau7.ClearCards();
	}

	void LoadDeck()
	{
		for (unsigned i=0; i<cardlist.size(); i++) 
			deck.AddCard(Card(cardlist[i].image, cardlist[i].suit, cardlist[i].rankstring, cardlist[i].color, cardlist[i].rank, deck.GetPosition()));
		Refresh();
	}
	
	void ShuffleDeck()
	{
		std::array<int,52> seq {0,1,2,3,4,5,6,7,8,9,
		10,11,12,13,14,15,16,17,18,19,
		20,21,22,23,24,25,26,27,28,29,
		30,31,32,33,34,35,36,37,38,39,
		40,41,42,43,44,45,46,47,48,49,
		50,51,
		};
		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		std::shuffle (seq.begin(), seq.end(), std::default_random_engine(seed));

		for (unsigned i=0; i<cardlist.size(); i++) {
			int j = seq[i];
			deck.AddCard(Card(cardlist[j].image, cardlist[j].suit, cardlist[j].rankstring, 
				cardlist[j].color, cardlist[j].rank, deck.GetPosition()));
		}
		Refresh();
	}

	bool CheckWin()
	{
		if (suit1.GetNumberOfCards() == 13 &
			suit2.GetNumberOfCards() == 13 &
			suit3.GetNumberOfCards() == 13 &
			suit4.GetNumberOfCards() == 13) {
				wins++;
				win = true;
				animate = true;
				t.Start(20,wxTIMER_ONE_SHOT);
				return true;
		}
		return false;
	}

	void OnAnimate(wxMouseEvent& event)
	{
		animate = true;
		t.Start(20,wxTIMER_ONE_SHOT);
	}
	
	void OnTimer(wxTimerEvent& event)
	{
		if (!animate) return;

		if (nextcard == 0) {  //starting...
			nextcard++;
			if (suit1.GetNumberOfCards() > 0) animatecard = suit1.ExtractTopCard();
			animatecard.vert = -15;
			animatecard.horiz = 10;
			oddeven = true;
		}
		else {
			int w, h;
			GetSize(&w, &h);
			wxPoint cp = animatecard.GetPosition();
			if (cp.x > w | cp.y > h | cp.x < 0 | cp.y < 0) {
				if (suit1.GetNumberOfCards() > 0) {
					animatecard = suit1.ExtractTopCard();
				}
				else if (suit2.GetNumberOfCards() > 0) {
					animatecard = suit2.ExtractTopCard();
				}
				else if (suit3.GetNumberOfCards() > 0) {
					animatecard = suit3.ExtractTopCard();
				}
				else if (suit4.GetNumberOfCards() > 0) {
					animatecard = suit4.ExtractTopCard();
				}
				else {
					animate = 0 ;
					return;
				}
				animatecard.vert = -15;
				if (oddeven)
					animatecard.horiz = -10;
				else
					animatecard.horiz = 10;
				oddeven = !oddeven;
			}

			wxPoint p = animatecard.GetPosition();
			animatecard.SetPosition(p.x + animatecard.horiz, p.y + animatecard.vert);
			animatecard.vert += 2;
		}
		t.Start(20,wxTIMER_ONE_SHOT);
		Refresh();
	}

private:
	Stack deck, movestack;
	
	Stack play, 
		suit1, suit2, suit3, 
		suit4, pile1, pile2, pile3, pile4, pile5, pile6, pile7, 
		tableau1, tableau2, tableau3, tableau4, tableau5, tableau6, tableau7,
		nullstack;
		
	Stack* originstack;
	std::string originstackname;
	int prevx, prevy;
	bool moving, animate, oddeven, win;
	
	int numberofcardstodraw;
	int wins, losses;
	
	wxTimer t;
	Card animatecard;
	int nextcard;

	myButton newgame1, newgame3;
	
};

enum {
	Minimal_Quit = wxID_EXIT,
	Minimal_About = wxID_ABOUT
};

class MyFrame : public wxFrame
{
public:
	MyFrame(const wxString& title): wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, wxSize(850,700))
	{

		wxMenu *fileMenu = new wxMenu;

		wxMenu *helpMenu = new wxMenu;
		helpMenu->Append(Minimal_About, "&About\tF1", "Show about dialog");
		fileMenu->Append(Minimal_Quit, "E&xit\tAlt-X", "Quit solitaire");

		wxMenuBar *menuBar = new wxMenuBar();
		menuBar->Append(fileMenu, "&File");
		menuBar->Append(helpMenu, "&Help");

		SetMenuBar(menuBar);

		cardpane = new CardPane(this, wxID_ANY);
		//cardpane->NewGame(3);

		CreateStatusBar();
		SetStatusText("Welcome to solitaire! Deal a new game with the buttons at top-left...");

	}
	
	void OnQuit(wxCommandEvent& WXUNUSED(event))
	{
		Close(true);
	}
	
	void OnAbout(wxCommandEvent& WXUNUSED(event))
	{
		wxMessageBox(wxT("This is a minimal Solitaire program.\n\nSource Code \u00A9 2021 Glenn Butcher, GPL 3.0 license, https://github.com/butcherg/solitaire.\nCard Deck \u00A9 2018 Howard Yeh, MIT license, https://github.com/hayeah/playing-cards-assets."),
                 "About Solitaire",
                 wxOK | wxICON_INFORMATION,
                 this);
	}

private:
	CardPane *cardpane;
	wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(MyFrame, wxFrame)
    EVT_MENU(Minimal_Quit,  MyFrame::OnQuit)
    EVT_MENU(Minimal_About, MyFrame::OnAbout)
wxEND_EVENT_TABLE()


class MyApp : public wxApp
{
public:
	bool OnInit()
	{
		if ( !wxApp::OnInit() )
		return false;

		MyFrame *frame = new MyFrame("Solitaire");
		frame->SetIcon(solitaire);
		frame->Show(true);
		return true;
	}
};

wxIMPLEMENT_APP(MyApp);


