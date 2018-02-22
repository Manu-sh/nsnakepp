#include <cstdio>

struct SaveGame {

	unsigned short speed{150}, score{0};
	unsigned char lv_food{5}, lv{1};

	SaveGame() noexcept {
		loadgame();
	}

	void nextlevel() noexcept {
		score   += lv_food;
		speed   -=(speed-2 > 0 ? 2 : 0); // ignore underflow, unreacheable for humans players
		lv_food +=(lv_food+5 < std::numeric_limits<unsigned char>::max() ? 5 : 0);
		lv++;
	}

	bool loadgame() noexcept { 

		FILE *file;
		unsigned short lv_food, lv;

		if (!(file = std::fopen("checkpoint.sav", "r")))
			return false;

		std::fscanf(file, "%hu\t%hu\t%hu\t%hu", &speed, &score, &lv_food, &lv);

		// workaround
		this->lv_food = (unsigned char)lv_food;
		this->lv      = (unsigned char)lv;

		std::fclose(file);
		return true;

	}

	bool savegame() noexcept {

		FILE *file;
		if (!(file = std::fopen("checkpoint.sav", "w+")))
			return false;

		std::fprintf(file, "%hu\t%hu\t%hu\t%hu", speed, score, lv_food, lv);
		std::fclose(file);
		return true;

	}

	void delsavegame() noexcept { remove("checkpoint.sav"); }
};
