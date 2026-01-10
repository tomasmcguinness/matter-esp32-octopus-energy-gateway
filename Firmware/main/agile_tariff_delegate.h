
#include <esp_err.h>
#include <esp_matter.h>
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/cluster-enums.h>
#include <app/clusters/commodity-tariff-server/commodity-tariff-server.h>

#pragma once

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;

namespace chip
{
    namespace app
    {
        namespace Clusters
        {
            namespace CommodityTariff
            {
                class AgileTariffDelegate : public Delegate
                {
                public:
                    AgileTariffDelegate();
                    ~AgileTariffDelegate() = default;
                };
            }
        }
    }
}
