#ifdef WITH_VST

#include "../src/core/pluginHost.h"
#include "catch.hpp"


TEST_CASE("Test PluginHost class")
{
  PluginHost ph;
  pthread_mutex_t mutex;
  pthread_mutex_init(&mutex, NULL);

  SECTION("test read & write")
  {
    REQUIRE(ph.scanDir(".") > 0);
    REQUIRE(ph.saveList("test-plugin-list.xml") == 1);
    REQUIRE(ph.loadList("test-plugin-list.xml") == 1);
    REQUIRE(ph.addPlugin(0, PluginHost::MASTER_IN, &mutex, 44100, 1024) != NULL);
    REQUIRE(ph.countPlugins(PluginHost::MASTER_IN) == 1);

    ph.freeStack(PluginHost::MASTER_IN, &mutex);
    REQUIRE(ph.countPlugins(PluginHost::MASTER_IN) == 0);
  }
}

#endif
