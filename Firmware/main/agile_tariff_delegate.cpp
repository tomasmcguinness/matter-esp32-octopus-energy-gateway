#include "agile_tariff_delegate.h"

using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;
using namespace chip::app::Clusters::Globals;
using namespace chip::app::DataModel;

static const char *TAG = "agile_tariff_delegate";

AgileTariffDelegate::AgileTariffDelegate()
{
    ESP_LOGI(TAG, "AgileTariffDelegate");

    // Tariff Info
    Structs::TariffInformationStruct::Type tariffInfo;
    tariffInfo.tariffLabel = DataModel::MakeNullable(CharSpan::fromCharString("Agile"));
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
    auto &dayEntriesAttr = GetDayEntries();

    // There are 48 30-minute slots in a 24 hour period day.
    static CommodityTariff::Structs::DayEntryStruct::Type dayEntries[48];

    for (uint32_t i = 0; i < 48; i++)
    {
        dayEntries[i].dayEntryID =  100 + i;
        dayEntries[i].startTime = i * 30;
        //dayEntries[i].duration = MakeOptional(DataModel::MakeNullable(30));
    }

    dayEntriesAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::DayEntryStruct::Type>(dayEntries, 48));

    // Day Patterns
    auto &dayPatternsAttr = GetDayPatterns();

    // Agile applies the same 48 period to each day of the week,
    // so we have one dayPattern (covering all days of the week), linked to each dayEntry.
    static CommodityTariff::Structs::DayPatternStruct::Type dayPatterns[1];
    dayPatterns[0].dayPatternID = 0x01;
    dayPatterns[0].daysOfWeek = static_cast<chip::BitMask<CommodityTariff::DayPatternDayOfWeekBitmap>>(0x7F);

    auto *dayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(48, sizeof(uint32_t)));

    for (uint32_t i = 0; i < 48; i++)
    {
        dayEntryIDs[i] = 100 + i;
    }

    dayPatterns[0].dayEntryIDs = DataModel::List<uint32_t>(dayEntryIDs, 48);

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
    auto &tariffComponentsAttr = GetTariffComponents();

    static CommodityTariff::Structs::TariffComponentStruct::Type tariffComponents[48];

    CommodityTariff::Structs::TariffPriceStruct::Type price;
    price.priceType = Globals::TariffPriceTypeEnum::kStandard;
    price.price = MakeOptional(0);

    for (uint32_t i = 0; i < 48; i++)
    {
        tariffComponents[i].tariffComponentID = 200 + i;
        tariffComponents[i].price = MakeOptional(DataModel::MakeNullable(price));
        tariffComponents[i].threshold = 0;
    }

    tariffComponentsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::TariffComponentStruct::Type>(tariffComponents, 48));

    // Tariff Periods
    auto &tariffPeriodsAttr = GetTariffPeriods();

    // We now need 48 tariff periods, one for each combination of dayEntry and tariffComponent
    static CommodityTariff::Structs::TariffPeriodStruct::Type tariffPeriods[48];
    
    // Connect each dayEntry with the corresponding tariff component.
    //
    for (uint8_t i = 0; i < 48; i++)
    {
        const char *label = i % 2 == 0 ? "Hour" : "Half Hour";
        tariffPeriods[i].label = DataModel::MakeNullable(CharSpan::fromCharString(label));

        dayEntryIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
        dayEntryIDs[0] = 100 + i;

        tariffPeriods[i].dayEntryIDs = DataModel::List<uint32_t>(dayEntryIDs, 1);

        auto *tariffComponentIDs = static_cast<uint32_t *>(Platform::MemoryCalloc(1, sizeof(uint32_t)));
        tariffComponentIDs[0] = 200 + i;

        tariffPeriods[i].tariffComponentIDs = DataModel::List<uint32_t>(tariffComponentIDs, 1);
    }

    tariffPeriodsAttr.SetNonNull(DataModel::List<CommodityTariff::Structs::TariffPeriodStruct::Type>(tariffPeriods, 48));
}
