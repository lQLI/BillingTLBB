#include "billing/net/packet/Routes.hpp"

#include "billing/database/models/Account.hpp"
#include "billing/net/packet/HexData.hpp"
#include "billing/Utils.hpp"
#include "billing/Log.hpp"

#include <utility>
#include <thread>

namespace net { namespace packet {
  Routes::Routes()
  {
    m_routers["A0"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onConnectHandle(hexData);
    };

    m_routers["A1"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onKeepLiveHandle(hexData);
    };

    m_routers["A2"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onLoginRequestHandle(hexData);
    };

    m_routers["A3"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onSelectCharHandle(hexData);
    };

    m_routers["A4"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onCharLogOutHandle(hexData);
    };

    m_routers["A6"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onWLBillingKeepHandle(hexData);
    };

    m_routers["A9"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onStartUpKickHandle(hexData);
    };

    // m_routers["C1"] = [this](const std::shared_ptr<packet::HexData> hexData)
    // ->ByteArray
    // {
    // };

    m_routers["C5"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onGWLWGCostLogHandle(hexData);
    };

    // m_routers["C6"] = [this](const std::shared_ptr<packet::HexData> hexData)
    // ->ByteArray
    // {
    // };

    m_routers["E1"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onAskPrizeAskBuyHandle(hexData);
    };

    m_routers["E2"] = [this](const std::shared_ptr<packet::HexData> hexData)
    ->ByteArray
    {
      return this->onAskPrizeAskPointHandle(hexData);
    };
  }

  Routes::~Routes()
  {
  }

  Routes& Routes::getInstance()
  {
    static Routes s_instance;
    return s_instance;
  }

  ByteArray Routes::operator[](const std::shared_ptr<Packet> hexData)
  {
    ByteArray m_responseData;

    for (const auto& router : m_routers)
    {
      if (hexData->getHexData()->getType() == router.first)
      {
        m_responseData = router.second(hexData->getHexData());
        break;
      }
    }

    return m_responseData;
  }

  // A0
  ByteArray Routes::onConnectHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    packet::HexData responseData;

    responseData.setType(hexData->getType());
    responseData.setId(hexData->getId());
    responseData.append("0100");

    return responseData.toByteArray();
  }

  // A1
  ByteArray Routes::onKeepLiveHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    packet::HexData responseData;
    responseData.setType(hexData->getType());
    responseData.setId(hexData->getId());
    responseData.append("0600");

    return Utils::hexToBytes(responseData.toString());
  }

  ByteArray Routes::onStartUpKickHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    packet::HexData responseData;
    responseData.setType("A1");
    responseData.setId(hexData->getId());
    responseData.append("0600");

    return responseData.toByteArray();
  }

#if defined(__BILLING_ENTERPRISE_EDITION__)
  // A2
#endif

#ifndef __BILLING_ENTERPRISE_EDITION__

  // A4

  // E1
  ByteArray Routes::onAskPrizeAskBuyHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    packet::HexData responseData;
    responseData.setType(hexData->getType());
    responseData.setId(hexData->getId());
    return responseData.toByteArray();
  }

#endif

  // E2
  ByteArray Routes::onAskPrizeAskPointHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    auto &packetHexStr = hexData->getBody();

    auto accountNameSizeHex = packetHexStr.substr(0, 2);
    auto accountNameSize = Utils::hexToNumber<std::size_t>(accountNameSizeHex);
    auto accountNameHex = packetHexStr.substr(
      accountNameSizeHex.size(),
      accountNameSize * 2
      );
    auto accountNameBytes = Utils::hexToBytes(accountNameHex);
    auto accountName = std::string(
      accountNameBytes.cbegin(),
      accountNameBytes.cend()
      );
    LOG->warning("Account name: {}", accountName);

    long long accountPoint = 0;

    try
    {
      database::models::Account a(accountName);

      accountPoint = a.getPoint() * 1000;
    }
    catch (const std::exception& e)
    {
      LOG->error("Sql database error: ", e.what());
    }
    catch (...)
    {
      LOG->error("Exception: Error code not found");
    }

    packet::HexData responseData;
    responseData.setType(hexData->getType());
    responseData.setId(hexData->getId());
    responseData.append(accountNameSizeHex);
    responseData.append(accountNameHex);
    responseData.append(Utils::numberToHex(accountPoint, 8));
    return responseData.toByteArray();
  }

  // A6
  ByteArray Routes::onWLBillingKeepHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    auto &packetHexStr = hexData->getBody();
    auto accountNameSizeHex = packetHexStr.substr(0, 2);
    auto accountNameSize = Utils::hexToNumber<std::size_t>(accountNameSizeHex);
    auto accountNameHex = packetHexStr.substr(
      accountNameSizeHex.size(),
      accountNameSize * 2
      );
    auto accountNameBytes = Utils::hexToBytes(accountNameHex);
    auto accountName = std::string(
      accountNameBytes.cbegin(),
      accountNameBytes.cend()
      );
    LOG->warning("Account name: {}", accountName);

    // TODO: Select Database

    packet::HexData responseData;
    responseData.setType(hexData->getType());
    responseData.setId(hexData->getId());
    responseData.append(accountNameSizeHex);
    responseData.append(accountNameHex);
    responseData.append("010000");
    return responseData.toByteArray();
  }

  // C5
  ByteArray Routes::onGWLWGCostLogHandle(
    const std::shared_ptr<packet::HexData> hexData
    ) const
  {
    LOG->warning(__FUNCTION__);

    // TODO: Fix this

    packet::HexData responseData;
    responseData.setType(hexData->getType());
    responseData.setId(hexData->getId());
    return responseData.toByteArray();
  }
} }

