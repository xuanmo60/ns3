/*
 * Copyright (c) 2015 LLNL
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Peter D. Barnes, Jr. <pdbarnes@llnl.gov>
 */

#include "ns3/build-profile.h"
#include "ns3/test.h"

/**
 * @file
 * @ingroup core-tests
 * @ingroup debugging
 * @ingroup build-profile-tests
 * NS_BUILD_PROFILE macros test suite.
 */

/**
 * @ingroup core-tests
 * @defgroup build-profile-tests NS_BUILD_PROFILE macros test suite
 */

namespace ns3
{

namespace tests
{

/**
 * @ingroup build-profile-tests
 * Build profile test
 */
class BuildProfileTestCase : public TestCase
{
  public:
    BuildProfileTestCase();

    ~BuildProfileTestCase() override
    {
    }

  private:
    void DoRun() override;
};

BuildProfileTestCase::BuildProfileTestCase()
    : TestCase("Check build profile macros")
{
}

void
BuildProfileTestCase::DoRun()
{
    int i = 0;
    int j = 0;

#ifdef NS3_BUILD_PROFILE_DEBUG
    std::cout << GetName() << ": running in build profile debug" << std::endl;
    NS_BUILD_DEBUG(++i; ++j);
#elif NS3_BUILD_PROFILE_RELEASE
    std::cout << GetName() << ": running in build profile release" << std::endl;
    NS_BUILD_RELEASE(++i; ++j);
#elif NS3_BUILD_PROFILE_OPTIMIZED
    std::cout << GetName() << ": running in build profile optimized" << std::endl;
    NS_BUILD_OPTIMIZED(++i; ++j);
#else
    NS_TEST_ASSERT_MSG_EQ(0, 1, ": no build profile case executed");
#endif

    if (i == 1)
    {
        std::cout << "build profile executed first statement." << std::endl;
    }
    NS_TEST_ASSERT_MSG_EQ(i, 1, "build profile failed to execute first statement");
    if (j == 1)
    {
        std::cout << "build profile executed second statement." << std::endl;
    }
    NS_TEST_ASSERT_MSG_EQ(j, 1, "build profile failed to execute second statement");
}

/**
 * @ingroup build-profile-tests
 * Build profile test suite
 */
class BuildProfileTestSuite : public TestSuite
{
  public:
    BuildProfileTestSuite();
};

BuildProfileTestSuite::BuildProfileTestSuite()
    : TestSuite("build-profile")
{
    AddTestCase(new BuildProfileTestCase);
}

/**
 * @ingroup build-profile-tests
 * BuildProfileTestSuite instance variable.
 */
static BuildProfileTestSuite g_BuildProfileTestSuite;

} // namespace tests

} // namespace ns3
