#include "display.hpp"
#include "font.hpp"
#include "map_label.hpp"

map_labels::map_labels(const display& disp) : disp_(disp)
{}

map_labels::map_labels(const display& disp, const config& cfg) : disp_(disp)
{
	read(cfg);
}

map_labels::~map_labels()
{
	clear();
}

config map_labels::write() const
{
	config res;
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		config item;
		i->first.write(item);
		item.values["text"] = get_label(i->first);
		res.add_child("label",item);
	}

	return res;
}

void map_labels::read(const config& cfg)
{
	clear();

	const config::child_list& items = cfg.get_children("label");
	for(config::child_list::const_iterator i = items.begin(); i != items.end(); ++i) {
		const gamemap::location loc(**i);
		const std::string& text = (**i)["text"];
		set_label(loc,text);
	}
}

const std::string& map_labels::get_label(const gamemap::location& loc) const
{
	const label_map::const_iterator itor = labels_.find(loc);
	if(itor != labels_.end()) {
		return font::get_floating_label_text(itor->second);
	} else {
		static const std::string empty_str;
		return empty_str;
	}
}

void map_labels::set_label(const gamemap::location& loc, const std::string& text)
{
	const label_map::iterator current_label = labels_.find(loc);
	if(current_label != labels_.end()) {
		font::remove_floating_label(current_label->second);
		labels_.erase(current_label);
	}

	if(text == "") {
		return;
	}

	SDL_Color colour = font::NORMAL_COLOUR;

	const gamemap::location loc_nextx(loc.x+1,loc.y);
	const gamemap::location loc_nexty(loc.x,loc.y+1);
	const int xloc = (disp_.get_location_x(loc) + disp_.get_location_x(loc_nextx)*2)/3;
	const int yloc = disp_.get_location_y(loc_nexty) - 14;
	const int handle = font::add_floating_label(text,14,colour,xloc,yloc,0,0,-1,disp_.map_area());

	labels_.insert(std::pair<gamemap::location,int>(loc,handle));
}

void map_labels::clear()
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::remove_floating_label(i->second);
	}

	labels_.clear();
}

void map_labels::scroll(double xmove, double ymove)
{
	for(label_map::const_iterator i = labels_.begin(); i != labels_.end(); ++i) {
		font::move_floating_label(i->second,int(xmove),int(ymove));
	}
}

void map_labels::recalculate_labels()
{
	const label_map labels = labels_;
	labels_.clear();
	for(label_map::const_iterator i = labels.begin(); i != labels.end(); ++i) {
		const std::string text = font::get_floating_label_text(i->second);
		font::remove_floating_label(i->second);
		set_label(i->first,text);
	}
}