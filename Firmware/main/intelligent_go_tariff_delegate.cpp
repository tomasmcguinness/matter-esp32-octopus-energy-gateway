#include "intelligent_go_tariff_delegate.h"

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::DataModel;

static const char *TAG = "intelligent_go_tariff_delegate";

IntelligentGoTariffDelegate::IntelligentGoTariffDelegate()
{
    ESP_LOGI(TAG, "IntelligentGoTariffDelegate");

    // Tariff Info
    Structs::TariffInformationStruct::Type tariffInfo;
    tariffInfo.tariffLabel = DataModel::MakeNullable(CharSpan::fromCharString("Intelligent Octopus Go"));
    tariffInfo.providerName = DataModel::MakeNullable(CharSpan::fromCharString("Octopus Energy"));

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
    //
    // Intelligent Octopus Go is a two-rate tariff with a fixed off-peak window of
    // 23:30-05:30 every night. Because that window crosses midnight, a calendar day is
    // split into three segments by start time:
    //   100 @ 0    (00:00-05:30) off-peak (tail of the previous night's window)
    //   101 @ 330  (05:30-23:30) peak
    //   102 @ 1410 (23:30-24:00) off-peak
    auto &dayEntriesAttr = GetDayEntries();

    static CommodityTariff::Structs::DayEntryStruct::Type dayEntries[3];

    dayEntries[0].dayEntryID = 100;
    dayEntries[0].startTime = 0;    // 00:00

    dayEntries[1].dayEntryID = 101;
    dayEntries[1].startTime = 330;  // 05:30

    dayEntries[2].dayEntryID = 102;
    dayEntries[2].startTime = 1410; // 23:30

    dayEntriesAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::DayEntryStruct::Type>(dayEntries, 3));

    // Day Patterns
    //
    // The same window applies to every day of the week, so we have one dayPattern
    // (covering all days), linked to all three dayEntries.
    auto &dayPatternsAttr = GetDayPatterns();

    static CommodityTariff::Structs::DayPatternStruct::Type dayPatterns[1];
    dayPatterns[0].dayPatternID = 0x01;
    dayPatterns[0].daysOfWeek = static_cast<chip::BitMask<CommodityTariff::DayPatternDayOfWeekBitmap>>(0x7F);

    auto *dayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(3, sizeof(uint32_t)));
    dayEntryIDs[0] = 100;
    dayEntryIDs[1] = 101;
    dayEntryIDs[2] = 102;

    dayPatterns[0].dayEntryIDs = DataModel::List<uint32_t>(dayEntryIDs, 3);

    dayPatternsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::DayPatternStruct::Type>(dayPatterns, 1));

    // Calendar Periods
    auto &calendarPeriodsAttr = GetCalendarPeriods();

    static CommodityTariff::Structs::CalendarPeriodStruct::Type calendarPeriods[1];
    calendarPeriods[0].startDate = 1767225600; // 1/1/2026

    auto *dayPatternIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    dayPatternIDs[0] = 0x01;

    calendarPeriods[0].dayPatternIDs = DataModel::List<uint32_t>(dayPatternIDs, 1);

    calendarPeriodsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::CalendarPeriodStruct::Type>(calendarPeriods, 1));

    // Tariff Components
    //
    // Two components: 200 = off-peak, 201 = peak. Prices start at 0 and are populated
    // by the fetch console command.
    auto &tariffComponentsAttr = GetTariffComponents();

    static CommodityTariff::Structs::TariffComponentStruct::Type tariffComponents[2];

    for (uint32_t i = 0; i < 2; i++)
    {
        CommodityTariff::Structs::TariffPriceStruct::Type price;
        price.priceType = Globals::TariffPriceTypeEnum::kStandard;
        price.price = MakeOptional(0);

        tariffComponents[i].tariffComponentID = 200 + i;
        tariffComponents[i].price = MakeOptional(DataModel::MakeNullable(price));
        tariffComponents[i].threshold = 0;
    }

    tariffComponentsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::TariffComponentStruct::Type>(tariffComponents, 2));

    // Tariff Periods
    //
    // One period per component, listing every dayEntry that uses it. The off-peak period
    // references both off-peak segments (100 and 102), which is how the midnight-crossing
    // window is expressed.
    auto &tariffPeriodsAttr = GetTariffPeriods();

    static CommodityTariff::Structs::TariffPeriodStruct::Type tariffPeriods[2];

    // Off Peak
    tariffPeriods[0].label = DataModel::MakeNullable(CharSpan::fromCharString("Off Peak"));

    auto *offPeakDayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(2, sizeof(uint32_t)));
    offPeakDayEntryIDs[0] = 100;
    offPeakDayEntryIDs[1] = 102;
    tariffPeriods[0].dayEntryIDs = DataModel::List<uint32_t>(offPeakDayEntryIDs, 2);

    auto *offPeakComponentIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    offPeakComponentIDs[0] = 200;
    tariffPeriods[0].tariffComponentIDs = DataModel::List<uint32_t>(offPeakComponentIDs, 1);

    // Peak
    tariffPeriods[1].label = DataModel::MakeNullable(CharSpan::fromCharString("Peak"));

    auto *peakDayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    peakDayEntryIDs[0] = 101;
    tariffPeriods[1].dayEntryIDs = DataModel::List<uint32_t>(peakDayEntryIDs, 1);

    auto *peakComponentIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
    peakComponentIDs[0] = 201;
    tariffPeriods[1].tariffComponentIDs = DataModel::List<uint32_t>(peakComponentIDs, 1);

    tariffPeriodsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::TariffPeriodStruct::Type>(tariffPeriods, 2));
}
