#include <iostream>
#include <fstream>
#include <algorithm>

#include "main.hpp"
#include "bsor.hpp"

// Simple program that reads a BSOR file and outputs info about it.

int main(int argc, char **argv) {
	if(argc == 1) {
		std::cout << "Usage: " << argv[0] << " <BSOR file> [events]" << std::endl;
		std::cout << "[events] is a sequence of letters, saying which types of events to show:" << std::endl;
		std::cout << "\tf - Frame\n\tn - Note\n\tw - Wall\n\th - Height\n\tp - Pause" << std::endl;
		return 0;
	}
	if(argc > 3) {
		std::cout << "Too many arguments.";
		return 1;
	}
	bool show_frame = false, show_note = false, show_wall = false, show_height = false, show_pause = false;
	if(argc == 3) {
		for(char ch : std::string(argv[2])) {
			switch(ch) {
				case 'f': show_frame = true; break;
				case 'n': show_note = true; break;
				case 'w': show_wall = true; break;
				case 'h': show_height = true; break;
				case 'p': show_pause = true; break;
				default:
					std::cout << "Unknown event type '" << ch << "'." << std::endl;
					return 1;
			}
		}
	}
	std::string file = argv[1];
	std::cout << "File " << file << ":" << std::endl;
	std::ifstream stream(file);
	auto result = bsor::read_bsor(stream);
	if(auto bsor_ptr = std::get_if<std::shared_ptr<bsor::BSOR>>(&result)) {
		auto bsor = *bsor_ptr;
		std::cout << "Info:" << std::endl;
		std::cout << "\tmod_version = " << bsor->info.mod_version << std::endl;
		std::cout << "\tgame_version = " << bsor->info.game_version << std::endl;
		std::cout << "\ttimestamp = " << bsor->info.timestamp << std::endl;
		std::cout << "\tplayer_id = " << bsor->info.player_id << std::endl;
		std::cout << "\tplayer_name = " << bsor->info.player_name << std::endl;
		std::cout << "\tplatform = " << bsor->info.platform << std::endl;
		std::cout << "\ttracking_system = " << bsor->info.tracking_system << std::endl;
		std::cout << "\thmd = " << bsor->info.hmd << std::endl;
		std::cout << "\tcontroller = " << bsor->info.controller << std::endl;
		std::cout << "\thash = " << bsor->info.hash << std::endl;
		std::cout << "\tsong_name = " << bsor->info.song_name << std::endl;
		std::cout << "\tmapper = " << bsor->info.mapper << std::endl;
		std::cout << "\tdifficulty = " << bsor->info.difficulty << std::endl;
		std::cout << "\tscore = " << bsor->info.score << std::endl;
		std::cout << "\tmode = " << bsor->info.mode << std::endl;
		std::cout << "\tenvironment = " << bsor->info.environment << std::endl;
		std::cout << "\tmodifiers:" << std::endl;
		for(auto &mod : bsor->info.modifiers) {
			std::cout << "\t\t" << mod << std::endl;
		}
		std::cout << "\tjump_distance = " << bsor->info.jump_distance << std::endl;
		std::cout << "\tleft_handed = " << bsor->info.left_handed << std::endl;
		std::cout << "\theight = " << bsor->info.height << std::endl;
		std::cout << "\tstart_time = " << bsor->info.start_time << std::endl;
		std::cout << "\tfail_time = " << bsor->info.fail_time << std::endl;
		std::cout << "\tsong_speed = " << bsor->info.song_speed << std::endl;
		std::vector<bsor::Event> filtered_events;
		std::copy_if(bsor->events.begin(), bsor->events.end(), std::back_inserter(filtered_events), [=](bsor::Event &evt) {
			return
				(show_frame && evt.type == bsor::EventType::FRAME) ||
				(show_note && evt.type == bsor::EventType::NOTE) ||
				(show_wall && evt.type == bsor::EventType::WALL) ||
				(show_height && evt.type == bsor::EventType::HEIGHT) ||
				(show_pause && evt.type == bsor::EventType::PAUSE);
		});
		if(filtered_events.size() == 0)
			return 0;
		std::cout << "Events:" << std::endl;
		size_t count = 0;
		size_t max_count = 20;
		for(auto &event : filtered_events) {
			switch(event.type) {
				case bsor::EventType::FRAME: {
					auto frame = event.frame;
					std::cout << "FRAME"
						<< "\n\ttime=" << frame.time
						<< "\n\tfps=" << frame.fps
						<< "\n\thead_pos=" << frame.head_pos
						<< "\n\thead_rot=" << frame.head_rot
						<< "\n\tleft_hand_pos=" << frame.left_hand_pos
						<< "\n\tleft_hand_rot=" << frame.left_hand_rot
						<< "\n\tright_hand_pos=" << frame.right_hand_pos
						<< "\n\tright_hand_rot=" << frame.right_hand_rot << std::endl;
					break;
				}
				case bsor::EventType::NOTE: {
					auto note = event.note;
					std::cout << "NOTE"
						<< "\n\ttime=" << note.time
						<< "\n\tid=" << note.id
						<< "\n\tspawn_time=" << note.spawn_time
						<< "\n\ttype=" << note.type;
					if(note.cut_data) {
						auto cd = note.cut_data.value();
						std::cout
							<< "\n\tspeed_ok=" << cd.speed_ok
							<< "\n\tdirection_ok=" << cd.direction_ok
							<< "\n\tsaber_type_ok=" << cd.saber_type_ok
							<< "\n\tcut_too_soon=" << cd.cut_too_soon
							<< "\n\tsaber_speed=" << cd.saber_speed
							<< "\n\tsaber_dir=" << cd.saber_dir
							<< "\n\tsaber_type=" << cd.saber_type
							<< "\n\ttime_deviation=" << cd.time_deviation
							<< "\n\tcut_dir_deviation=" << cd.cut_dir_deviation
							<< "\n\tcut_point=" << cd.cut_point
							<< "\n\tcut_normal=" << cd.cut_normal
							<< "\n\tcut_dist_to_center=" << cd.cut_dist_to_center
							<< "\n\tcut_angle=" << cd.cut_angle
							<< "\n\tbefore_cut_rating=" << cd.before_cut_rating
							<< "\n\tafter_cut_rating=" << cd.after_cut_rating << std::endl;	
					}
					break;
				}
				case bsor::EventType::WALL: {
					auto wall = event.wall;
					std::cout << "WALL"
						<< "\n\ttime=" << wall.time
						<< "\n\tid=" << wall.id
						<< "\n\tenergy=" << wall.energy
						<< "\n\tspawn_time=" << wall.spawn_time << std::endl;
					break;
				}
				case bsor::EventType::HEIGHT: {
					auto height = event.height;
					std::cout << "HEIGHT"
						<< "\n\ttime=" << height.time
						<< "\n\theight=" << height.height << std::endl;
					break;
				}
				case bsor::EventType::PAUSE: {
					auto pause = event.pause;
					std::cout << "PAUSE"
						<< "\n\ttime=" << pause.time
						<< "\n\tduration=" << pause.duration << std::endl;
					break;
				}
			}
			count++;
			if(count > max_count) {
				std::cout << "[" << filtered_events.size()-max_count << " more entries]" << std::endl;
				break;
			}
		}
	} else {
		std::cout << "\tParse failed: " << std::get<std::string>(result) << std::endl;
		return 1;
	}
	
	return 0;
}
