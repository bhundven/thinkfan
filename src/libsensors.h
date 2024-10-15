/********************************************************************
 * libsensors.h: State management for libsensors
 * (C) 2022, Victor Mataré
 *
 * this file is part of thinkfan. See thinkfan.c for further information.
 *
 * thinkfan is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * thinkfan is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with thinkfan.  If not, see <http://www.gnu.org/licenses/>.
 *
 * ******************************************************************/

#pragma once

#include "thinkfan.h"

#ifdef USE_LM_SENSORS
#include <sensors/sensors.h>
#include <sensors/error.h>
#include <map>

namespace thinkfan {

static const int MIN_CELSIUS_TEMP = -273;

class LMSensorsDriver;

class LibsensorsInterface
{
public:
	~LibsensorsInterface();
	LibsensorsInterface(const LibsensorsInterface &) = delete;
	LibsensorsInterface(LibsensorsInterface &&) = delete;

	static shared_ptr<LibsensorsInterface> instance();

	string lookup_client_features(LMSensorsDriver *client);
	vector<double> get_temps(LMSensorsDriver *client);

private:
	struct chip_features {
		const ::sensors_chip_name *chip = nullptr;
		vector<pair<const ::sensors_feature *, const ::sensors_subfeature *>> features;
	};

	/** @brief A scope guard to un-initialize libsensors when a requested feature/subfeature
	 * isn't found. This is necessary because libsensors doesn't pick up kernel drivers that
	 * are loaded after initialization. */
	class InitGuard {
	public:
		InitGuard(LMSensorsDriver *client);
		~InitGuard();
	private:
		LMSensorsDriver *client_;
		shared_ptr<LibsensorsInterface> iface_;
	};

	LibsensorsInterface();
	friend InitGuard;


	static void initialize_lm_sensors();

	// LM sensors call backs.
	static void parse_error_callback(const char *err, int line_no);
	static void parse_error_wfn_callback(const char *err, const char *file_name, int line_no);
	static void fatal_error_callback(const char *proc, const char *err);

	string get_chip_name(const ::sensors_chip_name &chip);

	const ::sensors_chip_name *find_chip_by_name(const string& chip_name);

	const ::sensors_feature *find_feature_by_name(
		const ::sensors_chip_name &chip,
		const string &feature_name
	);

	static std::weak_ptr<LibsensorsInterface> instance_;

	std::map<LMSensorsDriver *, chip_features> clients_;
	bool libsensors_initialized_;
};


}

#endif /* USE_LM_SENSORS */
