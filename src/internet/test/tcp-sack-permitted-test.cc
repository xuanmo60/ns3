/*
 * Copyright (c) 2016 Natale Patriciello <natale.patriciello@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 */

#include "tcp-general-test.h"

#include "ns3/log.h"
#include "ns3/node.h"
#include "ns3/tcp-header.h"
#include "ns3/tcp-option-sack-permitted.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SackPermittedTestSuite");

/**
 * @ingroup internet-test
 *
 * @brief Test case for checking the SACK-PERMITTED option.
 *
 */
class SackPermittedTestCase : public TcpGeneralTest
{
  public:
    /** @brief Configuration of the test */
    enum Configuration
    {
        DISABLED,
        ENABLED_RECEIVER,
        ENABLED_SENDER,
        ENABLED
    };

    /**
     * @brief Constructor
     * @param conf Test configuration.
     * */
    SackPermittedTestCase(SackPermittedTestCase::Configuration conf);

  protected:
    Ptr<TcpSocketMsgBase> CreateReceiverSocket(Ptr<Node> node) override;
    Ptr<TcpSocketMsgBase> CreateSenderSocket(Ptr<Node> node) override;

    void Tx(const Ptr<const Packet> p, const TcpHeader& h, SocketWho who) override;

    Configuration m_configuration; //!< The configuration
};

SackPermittedTestCase::SackPermittedTestCase(SackPermittedTestCase::Configuration conf)
    : TcpGeneralTest("Testing the TCP Sack Permitted option")
{
    m_configuration = conf;
}

Ptr<TcpSocketMsgBase>
SackPermittedTestCase::CreateReceiverSocket(Ptr<Node> node)
{
    Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateReceiverSocket(node);

    switch (m_configuration)
    {
    case DISABLED:
        socket->SetAttribute("Sack", BooleanValue(false));
        break;

    case ENABLED_RECEIVER:
        socket->SetAttribute("Sack", BooleanValue(true));
        break;

    case ENABLED_SENDER:
        socket->SetAttribute("Sack", BooleanValue(false));
        break;

    case ENABLED:
        socket->SetAttribute("Sack", BooleanValue(true));
        break;
    }

    return socket;
}

Ptr<TcpSocketMsgBase>
SackPermittedTestCase::CreateSenderSocket(Ptr<Node> node)
{
    Ptr<TcpSocketMsgBase> socket = TcpGeneralTest::CreateSenderSocket(node);

    switch (m_configuration)
    {
    case DISABLED:
    case ENABLED_RECEIVER:
        socket->SetAttribute("Sack", BooleanValue(false));
        break;

    case ENABLED_SENDER:
    case ENABLED:
        socket->SetAttribute("Sack", BooleanValue(true));
        break;
    }

    return socket;
}

void
SackPermittedTestCase::Tx(const Ptr<const Packet> p, const TcpHeader& h, SocketWho who)
{
    if (!(h.GetFlags() & TcpHeader::SYN))
    {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                              false,
                              "SackPermitted in non-SYN segment");
        return;
    }

    if (m_configuration == DISABLED)
    {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                              false,
                              "SackPermitted disabled but option enabled");
    }
    else if (m_configuration == ENABLED)
    {
        NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                              true,
                              "SackPermitted enabled but option disabled");
    }

    NS_LOG_INFO(h);
    if (who == SENDER)
    {
        if (h.GetFlags() & TcpHeader::SYN)
        {
            if (m_configuration == ENABLED_RECEIVER)
            {
                NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                                      false,
                                      "SackPermitted disabled but option enabled");
            }
            else if (m_configuration == ENABLED_SENDER)
            {
                NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                                      true,
                                      "SackPermitted enabled but option disabled");
            }
        }
        else
        {
            if (m_configuration != ENABLED)
            {
                NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                                      false,
                                      "SackPermitted disabled but option enabled");
            }
        }
    }
    else if (who == RECEIVER)
    {
        if (h.GetFlags() & TcpHeader::SYN)
        {
            // Sender has not sent SackPermitted, so implementation should disable ts
            if (m_configuration == ENABLED_RECEIVER)
            {
                NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                                      false,
                                      "sender has not ts, but receiver sent anyway");
            }
            else if (m_configuration == ENABLED_SENDER)
            {
                NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                                      false,
                                      "receiver has not ts enabled but sent anyway");
            }
        }
        else
        {
            if (m_configuration != ENABLED)
            {
                NS_TEST_ASSERT_MSG_EQ(h.HasOption(TcpOption::SACKPERMITTED),
                                      false,
                                      "SackPermitted disabled but option enabled");
            }
        }
    }
}

/**
 * @ingroup internet-test
 * @ingroup tests
 *
 * The test case for testing the TCP SACK PERMITTED option.
 */
class TcpSackPermittedTestSuite : public TestSuite
{
  public:
    /** @brief Constructor */
    TcpSackPermittedTestSuite()
        : TestSuite("tcp-sack-permitted", Type::UNIT)
    {
        AddTestCase(new SackPermittedTestCase(SackPermittedTestCase::DISABLED),
                    TestCase::Duration::QUICK);
        AddTestCase(new SackPermittedTestCase(SackPermittedTestCase::ENABLED_RECEIVER),
                    TestCase::Duration::QUICK);
        AddTestCase(new SackPermittedTestCase(SackPermittedTestCase::ENABLED_SENDER),
                    TestCase::Duration::QUICK);
        AddTestCase(new SackPermittedTestCase(SackPermittedTestCase::ENABLED),
                    TestCase::Duration::QUICK);
    }
};

static TcpSackPermittedTestSuite
    g_tcpSackPermittedTestSuite; //!< Static variable for test initialization
