#include "bsor.hpp"

#include <iostream>
#include <stdexcept>
#include <algorithm>

// The main BSOR parser (this is probably what you want to include in your project)

namespace bsor {
	static uint8_t read_byte(std::istream &stream) {
		char b;
		stream.read(&b, 1);
		return (uint8_t)b;
	}
	static bool read_bool(std::istream &stream) {
		return read_byte(stream) != 0;
	}
	static int32_t read_int(std::istream &stream) {
		uint8_t buf[4];
		stream.read(reinterpret_cast<char *>(buf), 4);
		return (int32_t)((uint32_t)buf[0] | ((uint32_t)buf[1] << 0x8) | ((uint32_t)buf[2] << 0x10) | ((uint32_t)buf[3] << 0x18));
	}
	static int64_t read_long(std::istream &stream) {
		uint8_t buf[8];
		stream.read(reinterpret_cast<char *>(buf), 8);
		return (int64_t)(
			(uint64_t)buf[0] | ((uint64_t)buf[1] << 0x8) | ((uint64_t)buf[2] << 0x10) | ((uint64_t)buf[3] << 0x18) |
			((uint64_t)buf[4] << 0x20) | ((uint64_t)buf[5] << 0x28) | ((uint64_t)buf[6] << 0x30) | ((uint64_t)buf[7] << 0x38)
		);
	}
	static uint32_t read_unsigned_int(std::istream &stream) {
		uint8_t buf[4];
		stream.read(reinterpret_cast<char *>(buf), 4);

		return (uint32_t)buf[0] | ((uint32_t)buf[1] << 0x8) | ((uint32_t)buf[2] << 0x10) | ((uint32_t)buf[3] << 0x18);
	}
	static float read_float(std::istream &stream) {
		int32_t i = read_int(stream);
		return *reinterpret_cast<float *>(&i);
	}
	static std::string read_string(std::istream &stream) {
		int32_t length = read_int(stream);
		char buf[length];
		stream.read(buf, length);
		return std::string(buf, length);
	}
	static Vector read_vector(std::istream &stream) {
		float x = read_float(stream);
		float y = read_float(stream);
		float z = read_float(stream);
		return {x, y, z};
	}
	static Quaternion read_quaternion(std::istream &stream) {
		float real = read_float(stream);
		float i = read_float(stream);
		float j = read_float(stream);
		float k = read_float(stream);
		return {real, i, j, k};
	}

	std::variant<std::shared_ptr<BSOR>, std::string> read_bsor(std::istream &stream) {
		auto ret = std::make_shared<BSOR>();
		try {
			// check magic number
			auto magic = read_unsigned_int(stream);
			if(magic != 0x442D3D69)
				return "Incorrect magic number.";
			auto version = read_byte(stream);
			if(version != 1)
				return "Unrecognized version number.";
			BSORInfo info;
			// read info
			if(stream.get() != 0x00)
				return "Header for info section should have byte 0x00.";
			info.mod_version = read_string(stream);
			info.game_version = read_string(stream);
			info.timestamp = std::stoi(read_string(stream));
			info.player_id = read_string(stream);
			info.player_name = read_string(stream);
			info.platform = read_string(stream);
			info.tracking_system = read_string(stream);
			info.hmd = read_string(stream);
			info.controller = read_string(stream);
			info.hash = read_string(stream);
			info.song_name = read_string(stream);
			info.mapper = read_string(stream);
			info.difficulty = read_string(stream);
			info.score = read_int(stream);
			info.mode = read_string(stream);
			info.environment = read_string(stream);
			{
				// get modifiers
				std::string modifiers_string = read_string(stream);
				std::vector<std::string> modifiers;
				size_t i, last = 0;
				// split by ,
				for(i = 0; i < modifiers_string.size(); i++) {
					if(modifiers_string[i] == ',') {
						modifiers.push_back(modifiers_string.substr(last, i-last));
						i += 2;
						last = i;
					}
				}
				if(i != last)
					modifiers.push_back(modifiers_string.substr(last, last-modifiers_string.size()));
				info.modifiers = modifiers;
			}
			info.jump_distance = read_float(stream);
			info.left_handed = read_bool(stream);
			info.height = read_float(stream);
			info.start_time = read_float(stream);
			info.fail_time = read_float(stream);
			info.song_speed = read_float(stream);
			ret->info = info;
			// read frame events
			if(read_byte(stream) != 0x01)
				return "Header for frames section should have byte 0x01.";
			{
				uint32_t count = read_unsigned_int(stream);
				for(uint32_t i = 0; i < count; i++) {
					FrameEvent event;
					event.time = read_float(stream);
					event.fps = read_int(stream);
					event.head_pos = read_vector(stream);
					event.head_rot = read_quaternion(stream);
					event.left_hand_pos = read_vector(stream);
					event.left_hand_rot = read_quaternion(stream);
					event.right_hand_pos = read_vector(stream);
					event.right_hand_rot = read_quaternion(stream);
					ret->events.push_back({.type = EventType::FRAME, .frame = event});
				}
			}
			// read note events
			if(read_byte(stream) != 0x02)
				return "Header for notes section should have byte 0x02.";
			{
				uint32_t count = read_unsigned_int(stream);
				for(uint32_t i = 0; i < count; i++) {
					NoteEvent event;
					event.id = read_int(stream);
					event.time = read_float(stream);
					event.spawn_time = read_float(stream);
					event.type = read_int(stream);
					if(event.type == NoteEvent::GOOD_HIT || event.type == NoteEvent::BAD_HIT) {
						CutData cut_data;
						cut_data.speed_ok = read_bool(stream);
						cut_data.direction_ok = read_bool(stream);
						cut_data.saber_type_ok = read_bool(stream);
						cut_data.cut_too_soon = read_bool(stream);
						cut_data.saber_speed = read_float(stream);
						cut_data.saber_dir = read_vector(stream);
						cut_data.saber_type = read_int(stream);
						cut_data.time_deviation = read_float(stream);
						cut_data.cut_dir_deviation = read_float(stream);
						cut_data.cut_point = read_vector(stream);
						cut_data.cut_normal = read_vector(stream);
						cut_data.cut_dist_to_center = read_float(stream);
						cut_data.cut_angle = read_float(stream);
						cut_data.before_cut_rating = read_float(stream);
						cut_data.after_cut_rating = read_float(stream);
						event.cut_data = cut_data;
					}
					ret->events.push_back({.type = EventType::NOTE, .note = event});
				}
			}
			// read wall events
			if(read_byte(stream) != 0x03)
				return "Header for walls section should have byte 0x03.";
			{
				uint32_t count = read_unsigned_int(stream);
				for(uint32_t i = 0; i < count; i++) {
					WallEvent event;
					event.id = read_int(stream);
					event.energy = read_float(stream);
					event.time = read_float(stream);
					event.spawn_time = read_float(stream);
					ret->events.push_back({.type = EventType::WALL, .wall = event});
				}
			}
			// read height events
			if(read_byte(stream) != 0x04)
				return "Header for height section should have byte 0x04.";
			{
				uint32_t count = read_unsigned_int(stream);
				for(uint32_t i = 0; i < count; i++) {
					HeightEvent event;
					event.height = read_float(stream);
					event.time = read_float(stream);
					ret->events.push_back({.type = EventType::HEIGHT, .height = event});
				}
			}
			// read pause events
			if(read_byte(stream) != 0x05)
				return "Header for pause section should have byte 0x05.";
			{
				uint32_t count = read_unsigned_int(stream);
				for(uint32_t i = 0; i < count; i++) {
					PauseEvent event;
					event.duration = read_long(stream);
					event.time = read_float(stream);
					ret->events.push_back({.type = EventType::PAUSE, .pause = event});
				}
			}
			// sort events
			std::sort(ret->events.begin(), ret->events.end(), [](Event &a, Event &b) { return a.time < b.time; });
		} catch(std::exception const &e) {
			return e.what();
		}
		return ret;
	}
}
