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
    dayPatterns[0].daysOfWeek = static_cast<chip::BitMask<CommodityTariff::DayPatternDayOfWeekBitmap>>(0x7F);

    auto *dayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    dayEntryIDs[0] = 0x19;
    dayPatterns[0].dayEntryIDs = DataModel::List<uint32_t>(dayEntryIDs, 1);

    dayPatternsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::DayPatternStruct::Type>(dayPatterns, 1));

    // Calendar Periods
    auto &calendarPeriodsAttr = GetCalendarPeriods();

    static CommodityTariff::Structs::CalendarPeriodStruct::Type calendarPeriods[1];
    calendarPeriods[0].startDate = 1767225600; // 1/1/2026

    auto *dayPatternIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    dayPatternIDs[0] = 0x15;
    
    calendarPeriods[0].dayPatternIDs = DataModel::List<uint32_t>(dayPatternIDs, 1);

    calendarPeriodsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::CalendarPeriodStruct::Type>(calendarPeriods, 1));

    // Tariff Components
    auto &tariffComponentsAttr = GetTariffComponents();

    static CommodityTariff::Structs::TariffComponentStruct::Type tariffComponents[1];
    tariffComponents[0].tariffComponentID = 0x29;

    CommodityTariff::Structs::TariffPriceStruct::Type price;
    price.priceType = Globals::TariffPriceTypeEnum::kStandard;
    price.price = MakeOptional(1000);
    
    tariffComponents[0].price = MakeOptional(DataModel::MakeNullable(price));
    tariffComponents[0].threshold = 0;

    tariffComponentsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::TariffComponentStruct::Type>(tariffComponents, 1));

    // Tariff Periods
    auto &tariffPeriodsAttr = GetTariffPeriods();

    static CommodityTariff::Structs::TariffPeriodStruct::Type tariffPeriods[1];
    tariffPeriods[0].label  = DataModel::MakeNullable(CharSpan::fromCharString("All Day"));

    dayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    dayEntryIDs[0] = 0x19;

    tariffPeriods[0].dayEntryIDs = DataModel::List<uint32_t>(dayEntryIDs, 1);

    auto *tariffComponentIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    tariffComponentIDs[0] = 0x29;

    tariffPeriods[0].tariffComponentIDs = DataModel::List<uint32_t>(tariffComponentIDs, 1);

    tariffPeriodsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::TariffPeriodStruct::Type>(tariffPeriods, 1));
}
