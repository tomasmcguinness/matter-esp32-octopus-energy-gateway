#include "simple_tariff_delegate.h"

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::DataModel;

static const char *TAG = "simple_tariff_delegate";

SimpleTariffDelegate::SimpleTariffDelegate()
{
    ESP_LOGI(TAG, "SimpleTariffDelegate");

    // Tariff Info
    Structs::TariffInformationStruct::Type tariffInfo;
    tariffInfo.tariffLabel = DataModel::MakeNullable(CharSpan::fromCharString("Simple Tariff"));
    tariffInfo.providerName = DataModel::MakeNullable(CharSpan::fromCharString("Watt's Up Power Co"));

    Globals::Structs::CurrencyStruct::Type currency;
    currency.currency = 826;
    currency.decimalPoints = 2;

    tariffInfo.currency = MakeOptional(DataModel::MakeNullable(currency));
    tariffInfo.blockMode = DataModel::MakeNullable(BlockModeEnum::kNoBlock);

    auto &tariffInfoAttr = GetTariffInfo();
    tariffInfoAttr.SetNonNull(tariffInfo);

    // Tariff Unit
    auto &tariffUnitAttr = GetTariffUnit();
    tariffUnitAttr.SetNonNull(TariffUnitEnum::kKWh);

    // Start Date
    auto &startDateAttr = GetStartDate();
    startDateAttr.SetNonNull(1767225600); // 1/1/2026

    // Day Entries
    auto &dayEntriesAttr = GetDayEntries();

    static CommodityTariff::Structs::DayEntryStruct::Type dayEntries[1];
    dayEntries[0].dayEntryID = 0x19;
    dayEntries[0].startTime = 0;

    dayEntriesAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::DayEntryStruct::Type>(dayEntries, 1));

    // Day Patterns
    auto &dayPatternsAttr = GetDayPatterns();

    static CommodityTariff::Structs::DayPatternStruct::Type dayPatterns[1];
    dayPatterns[0].dayPatternID = 0x15;
    dayPatterns[0].daysOfWeek = static_cast<chip::BitMask<DayPatternDayOfWeekBitmap>>(1);

    auto *dayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    dayEntryIDs[0] = 0x19;
    dayPatterns[0].dayEntryIDs = DataModel::List<uint32_t>(dayEntryIDs, 1);

    dayPatternsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::DayPatternStruct::Type>(dayPatterns, 1));
}
