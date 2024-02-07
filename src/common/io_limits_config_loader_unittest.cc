/*
   Copyright 2013-2014 EditShare
   Copyright 2013-2015 Skytechnology sp. z o.o.
   Copyright 2023      Leil Storage OÜ

   This file is part of SaunaFS.

   SaunaFS is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, version 3.

   SaunaFS is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with SaunaFS. If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/platform.h"
#include "common/io_limits_config_loader.h"

#include <sstream>
#include <gtest/gtest.h>

#include "common/exceptions.h"

#define PAIR(a, b) (std::make_pair(a, b))

#define LIMITS(...) (IoLimitsConfigLoader::LimitsMap({__VA_ARGS__}))
#define ASSERT_LIMITS_EQ(loader, ...) ASSERT_EQ(LIMITS(__VA_ARGS__), loader.limits())

TEST(IoLimitsConfigLoaderTest, CorrectFile) {
	std::string config(
			"subsystem lubie_placuszki\n"
			"limit a 1\n"
			"limit b 2\n"
			"limit c 3\n"
			"limit unclassified 4\n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_NO_THROW(loader.load(std::istringstream(config)));
	ASSERT_EQ("lubie_placuszki", loader.subsystem());
	ASSERT_LIMITS_EQ(loader, PAIR("a", 1), PAIR("b", 2), PAIR("c", 3), PAIR("unclassified", 4));
}

TEST(IoLimitsConfigLoaderTest, SubsystemNotSpecified1) {
	std::string config(
			"limit a 1\n"
			"limit b 2\n"
			"limit c 3\n"
			"limit unclassified 4\n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_THROW(loader.load(std::istringstream(config)), ParseException);
}

TEST(IoLimitsConfigLoaderTest, SubsystemNotSpecified2) {
	std::string config(
			"limit a 1\n"
			"limit b 2\n"
			"limit c 3\n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_THROW(loader.load(std::istringstream(config)), ParseException);
}

TEST(IoLimitsConfigLoaderTest, IncorrectLimit) {
	std::string config(
			"subsystem trololo\n"
			"limit a 1\n"
			"limit b cookie_monster\n"
			"limit c 3\n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_THROW(loader.load(std::istringstream(config)), ParseException);
}

TEST(IoLimitsConfigLoaderTest, UnknownKeyword) {
	std::string config(
			"subsystem trololo\n"
			"limit a 1\n"
			"limit b 45\n"
			"Agnieszka"
			"limit c 3\n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_THROW(loader.load(std::istringstream(config)), ParseException);
}

TEST(IoLimitsConfigLoaderTest, RepeatedGroup) {
	std::string config(
			"subsystem trololo\n"
			"limit a 1\n"
			"limit b 45\n"
			"limit a 3\n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_THROW(loader.load(std::istringstream(config)), ParseException);
}

TEST(IoLimitsConfigLoaderTest, Comment) {
	std::string config(
			"#\n"
			"subsystem trololo\n"
			"  #  limit a 1\n"
			"limit b 45\n"
			"#limit a 3\n"
			"  limit c 1#RANDOM_TEXT  \n"
			);
	IoLimitsConfigLoader loader;
	ASSERT_NO_THROW(loader.load(std::istringstream(config)));
	ASSERT_EQ("trololo", loader.subsystem());
	ASSERT_LIMITS_EQ(loader, PAIR("b", 45), PAIR("c", 1));
}

TEST(IoLimitsConfigLoaderTest, OnlyUnclassified) {
	std::string config("limit unclassified 1024\n");
	IoLimitsConfigLoader loader;
	ASSERT_NO_THROW(loader.load(std::istringstream(config)));
	ASSERT_EQ("", loader.subsystem());
	ASSERT_LIMITS_EQ(loader, PAIR("unclassified", 1024));
}
