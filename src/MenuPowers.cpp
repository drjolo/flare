/*
Copyright © 2011-2012 Clint Bellanger

This file is part of FLARE.

FLARE is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.

FLARE is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
FLARE.  If not, see http://www.gnu.org/licenses/
*/

/**
 * class MenuPowers
 */

#include "Menu.h"
#include "FileParser.h"
#include "MenuPowers.h"
#include "SharedResources.h"
#include "PowerManager.h"
#include "Settings.h"
#include "StatBlock.h"
#include "UtilsParsing.h"
#include "WidgetLabel.h"
#include "WidgetTooltip.h"

#include <string>
#include <sstream>

using namespace std;


MenuPowers::MenuPowers(StatBlock *_stats, PowerManager *_powers, SDL_Surface *_icons) {
	stats = _stats;
	powers = _powers;
	icons = _icons;

	visible = false;
	loadGraphics();

	points_left = 0;

	for (int i=0; i<20; i++) {
		power_cell[i].id = 0;
		power_cell[i].pos.x = 0;
		power_cell[i].pos.y = 0;
		power_cell[i].requires_mentdef = 0;
		power_cell[i].requires_mentoff = 0;
		power_cell[i].requires_physoff = 0;
		power_cell[i].requires_physdef = 0;
		power_cell[i].requires_defense = 0;
		power_cell[i].requires_offense = 0;
		power_cell[i].requires_physical = 0;
		power_cell[i].requires_mental = 0;
		power_cell[i].requires_level = 0;
		power_cell[i].requires_power = -1;
		power_cell[i].requires_point = false;

		plusButton[i] = new WidgetButton(mods->locate("images/menus/buttons/button_plus.png"));
	}

	closeButton = new WidgetButton(mods->locate("images/menus/buttons/button_x.png"));

	// Read powers data from config file 
	FileParser infile;
	int counter = -1;
	if (infile.open(mods->locate("menus/powers.txt"))) {
	  while (infile.next()) {
		infile.val = infile.val + ',';

		if (infile.key == "id") {
			counter++;
			power_cell[counter].id = eatFirstInt(infile.val, ',');
		} else if (infile.key == "position") {
			power_cell[counter].pos.x = eatFirstInt(infile.val, ',');
			power_cell[counter].pos.y = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_physoff") {
			power_cell[counter].requires_physoff = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_physdef") {
			power_cell[counter].requires_physdef = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_mentoff") {
			power_cell[counter].requires_mentoff = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_mentdef") {
			power_cell[counter].requires_mentdef = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_defense") {
			power_cell[counter].requires_defense = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_offense") {
			power_cell[counter].requires_offense = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_physical") {
			power_cell[counter].requires_physical = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_mental") {
			power_cell[counter].requires_mental = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_point") {
			if (infile.val == "true,") power_cell[counter].requires_point = true;
		} else if (infile.key == "requires_level") {
			power_cell[counter].requires_level = eatFirstInt(infile.val, ',');
		} else if (infile.key == "requires_power") {
			power_cell[counter].requires_power = eatFirstInt(infile.val, ',');
		} else if (infile.key == "closebutton_pos") {
			close_pos.x = eatFirstInt(infile.val, ',');
			close_pos.y = eatFirstInt(infile.val, ',');
		} else if (infile.key == "unspent_points_pos") {
			unspent_pos.x = eatFirstInt(infile.val, ',');
			unspent_pos.y = eatFirstInt(infile.val, ',');
		}

	  }
	} else fprintf(stderr, "Unable to open powers_menu.txt!\n");
	infile.close();

}

void MenuPowers::update() {
	for (int i=0; i<20; i++) {
		slots[i].w = slots[i].h = 32;
		slots[i].x = window_area.x + power_cell[i].pos.x;
		slots[i].y = window_area.y + power_cell[i].pos.y;

		plusButton[i]->pos.x = window_area.x + power_cell[i].pos.x + 32;
		plusButton[i]->pos.y = window_area.y + power_cell[i].pos.y;
	}

	label_powers.set(window_area.x+160, window_area.y+8, JUSTIFY_CENTER, VALIGN_TOP, msg->get("Powers"), FONT_WHITE);

	closeButton->pos.x = window_area.x+close_pos.x;
	closeButton->pos.y = window_area.y+close_pos.y;

	stat_up.set(window_area.x+unspent_pos.x, window_area.y+unspent_pos.y, JUSTIFY_CENTER, VALIGN_TOP, "", FONT_GREEN);
	points_left = stats->level - powers_list.size();
}

void MenuPowers::loadGraphics() {

	background = IMG_Load(mods->locate("images/menus/powers.png").c_str());
	powers_tree = IMG_Load(mods->locate("images/menus/powers_tree.png").c_str());
	powers_unlock = IMG_Load(mods->locate("images/menus/powers_unlock.png").c_str());
	if(!background || !powers_tree || !powers_unlock) {
		fprintf(stderr, "Couldn't load image: %s\n", IMG_GetError());
		SDL_Quit();
	}

	// optimize
	SDL_Surface *cleanup = background;
	background = SDL_DisplayFormatAlpha(background);
	SDL_FreeSurface(cleanup);

	cleanup = powers_tree;
	powers_tree = SDL_DisplayFormatAlpha(powers_tree);
	SDL_FreeSurface(cleanup);

	cleanup = powers_unlock;
	powers_unlock = SDL_DisplayFormatAlpha(powers_unlock);
	SDL_FreeSurface(cleanup);
}

/**
 * generic render 32-pixel icon
 */
void MenuPowers::renderIcon(int icon_id, int x, int y) {
	SDL_Rect icon_src;
	SDL_Rect icon_dest;

	icon_dest.x = x;
	icon_dest.y = y;
	icon_src.w = icon_src.h = icon_dest.w = icon_dest.h = 32;
	icon_src.x = (icon_id % 16) * 32;
	icon_src.y = (icon_id / 16) * 32;
	SDL_BlitSurface(icons, &icon_src, screen, &icon_dest);
}

/**
 * With great power comes great stat requirements.
 */
bool MenuPowers::requirementsMet(int power_index) {

	// power_index can be -1 during recursive call if requires_power is not defined.
	// Power with index -1 doesn't exist and is always enabled
	if (power_index == -1) return true;

	// Find cell with our power
	int id;
	for (int i=0; i<20; i++) {
		if (power_cell[i].id == power_index) {
		id = i;
		break;
		}
	}

	// If power_id is saved into vector, it's unlocked anyway
	if (find(powers_list.begin(), powers_list.end(), power_index) != powers_list.end()) return true;

	// Check the rest requirements
	if ((stats->physoff >= power_cell[id].requires_physoff) &&
		(stats->physdef >= power_cell[id].requires_physdef) &&
		(stats->mentoff >= power_cell[id].requires_mentoff) &&
		(stats->mentdef >= power_cell[id].requires_mentdef) &&
		(stats->get_defense() >= power_cell[id].requires_defense) &&
		(stats->get_offense() >= power_cell[id].requires_offense) &&
		(stats->get_physical() >= power_cell[id].requires_physical) &&
		(stats->get_mental() >= power_cell[id].requires_mental) &&
		(stats->level >= power_cell[id].requires_level) &&
		 requirementsMet(power_cell[id].requires_power) &&
		!power_cell[id].requires_point) return true;
	return false;
}

/**
 * Check if we can unlock power.
 */
bool MenuPowers::powerUnlockable(int power_index) {

	// power_index can be -1 during recursive call if requires_power is not defined.
	// Power with index -1 doesn't exist and is always enabled
	if (power_index == -1) return true;

	// Find cell with our power
	int id;
	for (int i=0; i<20; i++) {
		if (power_cell[i].id == power_index) {
		id = i;
		break;
		}
	}

	// Check requirements
	if ((stats->physoff >= power_cell[id].requires_physoff) &&
		(stats->physdef >= power_cell[id].requires_physdef) &&
		(stats->mentoff >= power_cell[id].requires_mentoff) &&
		(stats->mentdef >= power_cell[id].requires_mentdef) &&
		(stats->get_defense() >= power_cell[id].requires_defense) &&
		(stats->get_offense() >= power_cell[id].requires_offense) &&
		(stats->get_physical() >= power_cell[id].requires_physical) &&
		(stats->get_mental() >= power_cell[id].requires_mental) &&
		(stats->level >= power_cell[id].requires_level) &&
		 requirementsMet(power_cell[id].requires_power)) return true;
	return false;
}

/**
 * Click-to-drag a power (to the action bar)
 */
int MenuPowers::click(Point mouse) {

	for (int i=0; i<20; i++) {
		if (isWithin(slots[i], mouse)) {
			if (requirementsMet(power_cell[i].id)) return power_cell[i].id;
			else return -1;
		}
	}
	return -1;
}


void MenuPowers::logic() {
	if (!visible) return;

	if (closeButton->checkClick()) {
		visible = false;
	}
	for (int i=0; i<20; i++) {
		if (plusButton[i]->checkClick()) {
			powers_list.push_back(power_cell[i].id);
			points_left = stats->level - powers_list.size();
		}
	}
}

void MenuPowers::render() {
	if (!visible) return;

	SDL_Rect src;
	SDL_Rect dest;

	// background
	src.x = 0;
	src.y = 0;
	dest.x = window_area.x;
	dest.y = window_area.y;
	src.w = dest.w = 320;
	src.h = dest.h = 416;
	SDL_BlitSurface(background, &src, screen, &dest);
	SDL_BlitSurface(powers_tree, &src, screen, &dest);

	// power icons
	for (int i=0; i<20; i++) {
		bool power_in_vector = false;
		if (find(powers_list.begin(), powers_list.end(), power_cell[i].id) != powers_list.end()) power_in_vector = true;

		renderIcon(powers->powers[power_cell[i].id].icon, window_area.x + power_cell[i].pos.x, window_area.y + power_cell[i].pos.y);

		// Disable all buttons
		plusButton[i]->enabled = false;

		// Enable button if we can unlock power
		if (power_cell[i].requires_point && !power_in_vector && powerUnlockable(power_cell[i].id) && (points_left > 0)) {
			plusButton[i]->enabled = true;
			plusButton[i]->render();
		}

		// highlighting
		if (power_in_vector || requirementsMet(power_cell[i].id))
			displayBuild(power_cell[i].id);
	}

	// close button
	closeButton->render();

	// text overlay
	label_powers.render();

	// stats
	stringstream ss;

	ss.str("");
	points_left = stats->level - powers_list.size();
	if (points_left !=0) {
		ss << "Unspent power points:" << " " << points_left;
	}
	stat_up.set(ss.str());
	stat_up.render();
}

/**
 * Highlight unlocked powers
 */
void MenuPowers::displayBuild(int power_id) {
	SDL_Rect src_unlock;
	SDL_Rect dest;

	src_unlock.x = 0;
	src_unlock.y = 0;
	src_unlock.w = 32;
	src_unlock.h = 32;

	for (int i=0; i<20; i++) {
		if (power_cell[i].id == power_id) {
			dest.x = window_area.x + power_cell[i].pos.x;
			dest.y = window_area.y + power_cell[i].pos.y;
			SDL_BlitSurface(powers_unlock, &src_unlock, screen, &dest);
		}
	}
}

/**
 * Show mouseover descriptions of disciplines and powers
 */
TooltipData MenuPowers::checkTooltip(Point mouse) {

	TooltipData tip;

		for (int i=0; i<20; i++) {
			if (isWithin(slots[i], mouse)) {
				tip.lines[tip.num_lines++] = powers->powers[power_cell[i].id].name;
				tip.lines[tip.num_lines++] = powers->powers[power_cell[i].id].description;

				if (powers->powers[power_cell[i].id].requires_physical_weapon)
					tip.lines[tip.num_lines++] = msg->get("Requires a physical weapon");
				else if (powers->powers[power_cell[i].id].requires_mental_weapon)
					tip.lines[tip.num_lines++] = msg->get("Requires a mental weapon");
				else if (powers->powers[power_cell[i].id].requires_offense_weapon)
					tip.lines[tip.num_lines++] = msg->get("Requires an offense weapon");


				// add requirement
				int required_val = (i / 4) * 2 + 1;
				int required_stat = i % 4;
				if (required_val > 1) {

					if (!requirementsMet(i))
						tip.colors[tip.num_lines] = FONT_RED;

					if (required_stat == 0) tip.lines[tip.num_lines++] = msg->get("Requires Physical Offense %d", required_val);
					else if (required_stat == 1) tip.lines[tip.num_lines++] = msg->get("Requires Physical Defense %d", required_val);
					else if (required_stat == 2) tip.lines[tip.num_lines++] = msg->get("Requires Mental Offense %d", required_val);
					else tip.lines[tip.num_lines++] = msg->get("Requires Mental Defense %d", required_val);

				}

				// add mana cost
				if (powers->powers[power_cell[i].id].requires_mp > 0) {
					tip.lines[tip.num_lines++] = msg->get("Costs %d MP", powers->powers[power_cell[i].id].requires_mp);
				}
				// add cooldown time
				if (powers->powers[power_cell[i].id].cooldown > 0) {
					tip.lines[tip.num_lines++] = msg->get("Cooldown: %d seconds", powers->powers[power_cell[i].id].cooldown / 1000.0);
				}

				return tip;
			}
		}

	return tip;
}

MenuPowers::~MenuPowers() {
	SDL_FreeSurface(background);
	SDL_FreeSurface(powers_unlock);
	delete closeButton;
	for (int i=0; i<20; i++) {
	delete plusButton[i];
	}
}
