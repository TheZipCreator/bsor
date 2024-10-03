#include <ctime>
#include <cstdint>
#include <istream>
#include <vector>
#include <memory>
#include <variant>
#include <optional>

// The main BSOR parser (this is probably what you want to include in your project)
// see the BSOR documentation for what the field names mean

namespace bsor {
	// Vector
	struct Vector {
		float x, y, z;
	};
	// Quaternion
	struct Quaternion {
		float real, i, j, k;
	};

	static inline std::ostream & operator<<(std::ostream &stream, const Vector &vec) {
		return stream << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
	}
	static inline std::ostream & operator<<(std::ostream &stream, const Quaternion &quat) {
		return stream << "(" << quat.real << ", " << quat.i << ", " << quat.j << ", " << quat.k << ")";
	}

	// BSOR info
	struct BSORInfo {
		std::string mod_version;
		std::string game_version;
		std::time_t timestamp;
		
		std::string player_id;
		std::string player_name;
		std::string platform;
		
		std::string tracking_system;
		std::string hmd;
		std::string controller;

		std::string hash;
		std::string song_name;
		std::string mapper;
		std::string difficulty;

		int32_t score;
		std::string mode;
		std::string environment;
		std::vector<std::string> modifiers;
		float jump_distance;
		bool left_handed;
		float height;

		float start_time;
		float fail_time;
		float song_speed;
	};

	// frame event
	struct FrameEvent {
		float time;
		int32_t fps;
		Vector head_pos;
		Quaternion head_rot;
		Vector left_hand_pos;
		Quaternion left_hand_rot;
		Vector right_hand_pos;
		Quaternion right_hand_rot;
	};

	// Note cut data
	struct CutData {
		bool speed_ok;
		bool direction_ok;
		bool saber_type_ok;
		bool cut_too_soon;
		float saber_speed;
		Vector saber_dir;
		int32_t saber_type;
		float time_deviation;
		float cut_dir_deviation;
		Vector cut_point;
		Vector cut_normal;
		float cut_dist_to_center;
		float cut_angle;
		float before_cut_rating;
		float after_cut_rating;
	};

	// Note event
	struct NoteEvent {
		static constexpr int32_t GOOD_HIT = 0;
		static constexpr int32_t BAD_HIT = 1;
		static constexpr int32_t MISS = 2;
		static constexpr int32_t BOMB = 3;
		
		float time;
		int32_t id;
		float spawn_time;
		int32_t type;
		std::optional<CutData> cut_data = std::nullopt;
	};

	// Wall event
	struct WallEvent {
		float time;
		int id;
		float energy;
		float spawn_time;
	};

	// Height event
	struct HeightEvent {
		float time;
		float height;
	};
	
	// Pause event
	struct PauseEvent {
		float time;
		uint64_t duration;
	};

	enum EventType : uint8_t {
		FRAME, NOTE, WALL, HEIGHT, PAUSE
	};

	// An event
	struct Event {
		uint8_t type;
		union {
			float time;
			FrameEvent frame;
			NoteEvent note;
			WallEvent wall;
			HeightEvent height;
			PauseEvent pause;
		};
	};

	// BSOR
	class BSOR {
		public:
		BSORInfo info;
		std::vector<Event> events;
	};
	
	// returns a BSOR pointer on success, or a string containing an error on failure
	std::variant<std::shared_ptr<BSOR>, std::string> read_bsor(std::istream &stream);
}
