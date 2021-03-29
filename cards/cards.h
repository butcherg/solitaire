#ifndef __CARDS__
#define __CARDS__

#include <vector>
#include <string>

#include "ace_clubs.xpm"
#include "ace_diamonds.xpm"
#include "ace_hearts.xpm"
#include "ace_spades.xpm"
#include "black_.xpm"
#include "eight_clubs.xpm"
#include "eight_diamonds.xpm"
#include "eight_hearts.xpm"
#include "eight_spades.xpm"
#include "five_clubs.xpm"
#include "five_diamonds.xpm"
#include "five_hearts.xpm"
#include "five_spades.xpm"
#include "four_clubs.xpm"
#include "four_diamonds.xpm"
#include "four_hearts.xpm"
#include "four_spades.xpm"
#include "jack_clubs.xpm"
#include "jack_diamonds.xpm"
#include "jack_hearts.xpm"
#include "jack_spades.xpm"
#include "king_clubs.xpm"
#include "king_diamonds.xpm"
#include "king_hearts.xpm"
#include "king_spades.xpm"
#include "nine_clubs.xpm"
#include "nine_diamonds.xpm"
#include "nine_hearts.xpm"
#include "nine_spades.xpm"
#include "queen_clubs.xpm"
#include "queen_diamonds.xpm"
#include "queen_hearts.xpm"
#include "queen_spades.xpm"
#include "red_.xpm"
#include "seven_clubs.xpm"
#include "seven_diamonds.xpm"
#include "seven_hearts.xpm"
#include "seven_spades.xpm"
#include "six_clubs.xpm"
#include "six_diamonds.xpm"
#include "six_hearts.xpm"
#include "six_spades.xpm"
#include "ten_clubs.xpm"
#include "ten_diamonds.xpm"
#include "ten_hearts.xpm"
#include "ten_spades.xpm"
#include "three_clubs.xpm"
#include "three_diamonds.xpm"
#include "three_hearts.xpm"
#include "three_spades.xpm"
#include "two_clubs.xpm"
#include "two_diamonds.xpm"
#include "two_hearts.xpm"
#include "two_spades.xpm"

struct card_rec {
	const char ** image;
	std::string rankstring;
	std::string suit;
	std::string color;
	int rank;
};

//const char **cardlist[] = {
//std::vector<const char **> cardlist = {
std::vector<card_rec> cardlist = {
	{ace_clubs,"ace","clubs","black",1},
	{ace_diamonds,"ace","diamonds","red",1},
	{ace_hearts,"ace","hearts","red",1},
	{ace_spades,"ace","spades","black",1},
	{eight_clubs,"eight","clubs","black",8},
	{eight_diamonds,"eight","diamonds","red",8},
	{eight_hearts,"eight","hearts","red",8},
	{eight_spades,"eight","spades","black",8},
	{five_clubs,"five","clubs","black",5},
	{five_diamonds,"five","diamonds","red",5},
	{five_hearts,"five","hearts","red",5},
	{five_spades,"five","spades","black",5},
	{four_clubs,"four","clubs","black",4},
	{four_diamonds,"four","diamonds","red",4},
	{four_hearts,"four","hearts","red",4},
	{four_spades,"four","spades","black",4},
	{jack_clubs,"jack","clubs","black",11},
	{jack_diamonds,"jack","diamonds","red",11},
	{jack_hearts,"jack","hearts","red",11},
	{jack_spades,"jack","spades","black",11},
	{king_clubs,"king","clubs","black",13},
	{king_diamonds,"king","diamonds","red",13},
	{king_hearts,"king","hearts","red",13},
	{king_spades,"king","spades","black",13},
	{nine_clubs,"nine","clubs","black",9},
	{nine_diamonds,"nine","diamonds","red",9},
	{nine_hearts,"nine","hearts","red",9},
	{nine_spades,"nine","spades","black",9},
	{queen_clubs,"queen","clubs","black",12},
	{queen_diamonds,"queen","diamonds","red",12},
	{queen_hearts,"queen","hearts","red",12},
	{queen_spades,"queen","spades","black",12},
	{seven_clubs,"seven","clubs","black",7},
	{seven_diamonds,"seven","diamonds","red",7},
	{seven_hearts,"seven","hearts","red",7},
	{seven_spades,"seven","spades","black",7},
	{six_clubs,"six","clubs","black",6},
	{six_diamonds,"six","diamonds","red",6},
	{six_hearts,"six","hearts","red",6},
	{six_spades,"six","spades","black",6},
	{ten_clubs,"ten","clubs","black",10},
	{ten_diamonds,"ten","diamonds","red",10},
	{ten_hearts,"ten","hearts","red",10},
	{ten_spades,"ten","spades","black",10},
	{three_clubs,"three","clubs","black",3},
	{three_diamonds,"three","diamonds","red",3},
	{three_hearts,"three","hearts","red",3},
	{three_spades,"three","spades","black",3},
	{two_clubs,"two","clubs","black",2},
	{two_diamonds,"two","diamonds","red",2},
	{two_hearts,"two","hearts","red",2},
	{two_spades,"two","spades","black",2}
};

#endif

