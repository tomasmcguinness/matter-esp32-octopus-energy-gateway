#include <esp_err.h>
#include <esp_log.h>
#include "nvs_flash.h"
#include "nvs.h"

#include <esp_matter.h>
#include <common_macros.h>

#include <string>

#include <esp_matter.h>
#include <lib/dnssd/Types.h>

#include "cJSON.h"

#include "app_main.h"

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace chip;
using namespace chip::app::Clusters;

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type)
    {
    case chip::DeviceLayer::DeviceEventType::PublicEventTypes::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "kInterfaceIpAddressChanged");
        break;
    case chip::DeviceLayer::DeviceEventType::kESPSystemEvent:
        ESP_LOGI(TAG, "kESPSystemEvent");
        break;
    default:
        break;
    }
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGE(TAG, "nvs_flash_init error");
    }
    ESP_ERROR_CHECK(err);

    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));
}
