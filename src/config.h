/********************************************************************
 * config.h: Config data structures and consistency checking.
 * (C) 2015, Victor Mataré
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

#ifndef THINKFAN_CONFIG_H_
#define THINKFAN_CONFIG_H_

#include "temperature_state.h"

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "thinkfan.h"

namespace thinkfan {

class FanConfig {
public:
	FanConfig(unique_ptr<FanDriver> && = nullptr);
	virtual ~FanConfig() = default;
	virtual void init_fanspeed(const TemperatureState &) = 0;
	virtual bool set_fanspeed(const TemperatureState &) = 0;
	virtual void ensure_consistency(const Config &) const = 0;
	void set_fan(unique_ptr<FanDriver> &&);
	const unique_ptr<FanDriver> &fan() const;

private:
	unique_ptr<FanDriver> fan_;
};


class StepwiseMapping : public FanConfig {
public:
	StepwiseMapping(unique_ptr<FanDriver> && = nullptr);
	virtual ~StepwiseMapping() override = default;
	virtual void init_fanspeed(const TemperatureState &) override;
	virtual bool set_fanspeed(const TemperatureState &) override;
	virtual void ensure_consistency(const Config &) const override;
	void add_level(unique_ptr<Level> &&level);
	const vector<unique_ptr<Level>> &levels() const;

private:
	vector<unique_ptr<Level>> levels_;
	vector<unique_ptr<Level>>::const_iterator cur_lvl_;
};


class Level {
protected:
	string level_s_;
	int level_n_;
	vector<int> lower_limit_;
	vector<int> upper_limit_;
public:
	Level(int level, int lower_limit, int upper_limit);
	Level(string level, int lower_limit, int upper_limit);
	Level(int level, const vector<int> &lower_limit, const vector<int> &upper_limit);
	Level(string level, const vector<int> &lower_limit, const vector<int> &upper_limit);

	virtual ~Level() = default;

	const vector<int> &lower_limit() const;
	const vector<int> &upper_limit() const;

	virtual bool up(const TemperatureState &) const = 0;
	virtual bool down(const TemperatureState &) const = 0;

	virtual void ensure_consistency(const Config &) const = 0;

	const string &str() const;
	int num() const;

	static int string_to_int(string &level);
};



class SimpleLevel : public Level {
public:
	SimpleLevel(int level, int lower_limit, int upper_limit);
	SimpleLevel(string level, int lower_limit, int upper_limit);
	virtual bool up(const TemperatureState &) const override;
	virtual bool down(const TemperatureState &) const override;
	virtual void ensure_consistency(const Config &) const override;
};


class ComplexLevel : public Level {
public:
	ComplexLevel(int level, const vector<int> &lower_limit, const vector<int> &upper_limit);
	ComplexLevel(string level, const vector<int> &lower_limit, const vector<int> &upper_limit);
	virtual bool up(const TemperatureState &) const override;
	virtual bool down(const TemperatureState &) const override;
	virtual void ensure_consistency(const Config &) const override;

private:
	static string format_limit(const vector<int> &limit);
};


class Config {
public:
	Config() = default;
	~Config() = default;

	static const Config *read_config(const vector<string> &filenames);
	void add_sensor(unique_ptr<SensorDriver> &&sensor);
	void add_fan_config(unique_ptr<FanConfig> &&fan_cfg);
	void ensure_consistency() const;
	void init_fans() const;
	TemperatureState init_sensors() const;
	void init_temperature_refs(TemperatureState &tstate) const;
	void init(TemperatureState &ts) const;

	unsigned int num_temps() const;
	const vector<unique_ptr<SensorDriver>> &sensors() const;
	const vector<unique_ptr<FanConfig>> &fan_configs() const;

	string src_file;
private:
	static const Config *try_read_config(const string &data);
	void try_init_driver(Driver &drv) const;
	vector<unique_ptr<SensorDriver>> sensors_;
	vector<unique_ptr<FanConfig>> temp_mappings_;
};


}

#endif /* THINKFAN_CONFIG_H_ */
