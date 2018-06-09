#include "../src/core/sampleChannel.h"
#include "../src/core/wave.h"
#include "../src/core/waveManager.h"
#include <catch.hpp>


using namespace giada::m;


TEST_CASE("sampleChannel")
{
	const int BUFFER_SIZE = 1024;

	std::vector<ChannelMode> modes = { ChannelMode::LOOP_BASIC, 
		ChannelMode::LOOP_ONCE, ChannelMode::LOOP_REPEAT, 
		ChannelMode::LOOP_ONCE_BAR, ChannelMode::SINGLE_BASIC, 
		ChannelMode::SINGLE_PRESS, ChannelMode::SINGLE_RETRIG, 
		ChannelMode::SINGLE_ENDLESS };

	SampleChannel ch(false, BUFFER_SIZE);
	Wave* w;
	waveManager::create("tests/resources/test.wav", &w); 
	ch.pushWave(w);

	SECTION("push wave")
	{
		REQUIRE(ch.status == ChannelStatus::OFF);
		REQUIRE(ch.wave == w);
		REQUIRE(ch.begin == 0);
		REQUIRE(ch.end == w->getSize() - 1);
		REQUIRE(ch.name == w->getBasename());		
	}

	SECTION("begin/end")
	{
		ch.setBegin(-100);

		REQUIRE(ch.getBegin() == 0);
		REQUIRE(ch.tracker == 0);
		REQUIRE(ch.trackerPreview == 0);

		ch.setBegin(100000);

		REQUIRE(ch.getBegin() == w->getSize());
		REQUIRE(ch.tracker == w->getSize());
		REQUIRE(ch.trackerPreview == w->getSize());

		ch.setBegin(16);

		REQUIRE(ch.getBegin() == 16);
		REQUIRE(ch.tracker == 16);
		REQUIRE(ch.trackerPreview == 16);

		ch.setEnd(0);

		REQUIRE(ch.getEnd() == 17);

		ch.setEnd(100000);

		REQUIRE(ch.getEnd() == w->getSize() - 1);

		ch.setEnd(32);

		REQUIRE(ch.getEnd() == 32);

		ch.setBegin(64);

		REQUIRE(ch.getBegin() == 31);
	}

	SECTION("pitch")
	{
		ch.setPitch(40.0f);

		REQUIRE(ch.getPitch() == G_MAX_PITCH);

		ch.setPitch(-2.0f);

		REQUIRE(ch.getPitch() == G_MIN_PITCH);

		ch.setPitch(0.8f);

		REQUIRE(ch.getPitch() == 0.8f);
	}

	SECTION("position")
	{
		REQUIRE(ch.getPosition() == -1);  // Initially OFF

		ch.status  = ChannelStatus::PLAY;
		ch.tracker = 1000;

		REQUIRE(ch.getPosition() == 1000);

		ch.begin = 700;

		REQUIRE(ch.getPosition() == 300);
	}

	SECTION("empty")
	{
		ch.empty();

		REQUIRE(ch.status == ChannelStatus::EMPTY);
		REQUIRE(ch.begin == 0);
		REQUIRE(ch.end == 0);
		REQUIRE(ch.tracker == 0);
		REQUIRE(ch.volume == G_DEFAULT_VOL);
		REQUIRE(ch.boost == G_DEFAULT_BOOST);
		REQUIRE(ch.hasActions == false);
		REQUIRE(ch.wave == nullptr);
	}

	SECTION("can record audio")
	{	
		REQUIRE(ch.canInputRec() == false); // Can't record if not armed

		ch.armed = true;

		REQUIRE(ch.canInputRec() == false); // Can't record with Wave in it

		ch.empty();

		REQUIRE(ch.canInputRec() == true);
	}

	/* TODO - fillBuffer, isAnyLoopMode, isAnySingleMode, isOnLastFrame */
}
