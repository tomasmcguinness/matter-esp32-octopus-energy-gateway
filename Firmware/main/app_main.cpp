#include <esp_err.h>
#include <esp_log.h>
#include "nvs_flash.h"
#include "nvs.h"

#include <esp_matter.h>
#include <common_macros.h>

#include <string>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <lib/dnssd/Types.h>

#include "cJSON.h"

#include "app_main.h"

#include <setup_payload/OnboardingCodesUtil.h>
#include <platform/CommissionableDataProvider.h>
#include <platform/ESP32/ESP32Config.h>

#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_http_client.h"
#include "esp_crt_bundle.h"

#include "cJSON.h"

#include "simple_tariff_delegate.h"
#include "agile_tariff_delegate.h"

static const char *TAG = "app_main";

using namespace esp_matter;
using namespace chip;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::CommodityTariff;

static AgileTariffDelegate commodity_tariff_delegate;

void start_matter();

#define MAX_HTTP_RECV_BUFFER 1024
#define MAX_HTTP_OUTPUT_BUFFER 20000

esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    static char *output_buffer; // Buffer to store response of http request from event handler
    static int output_len;      // Stores number of bytes read

    switch (evt->event_id)
    {
    case HTTP_EVENT_ERROR:
        ESP_LOGI(TAG, "HTTP_EVENT_ERROR");
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_CONNECTED");
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGI(TAG, "HTTP_EVENT_HEADER_SENT");
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
        // Clean the buffer in case of a new request
        if (output_len == 0 && evt->user_data)
        {
            // we are just starting to copy the output data into the use
            memset(evt->user_data, 0, MAX_HTTP_OUTPUT_BUFFER);
        }
        /*
         *  Check for chunked encoding is added as the URL for chunked encoding used in this example returns binary data.
         *  However, event handler can also be used in case chunked encoding is used.
         */
        if (!esp_http_client_is_chunked_response(evt->client))
        {
            // If user_data buffer is configured, copy the response into the buffer
            int copy_len = 0;
            if (evt->user_data)
            {
                // The last byte in evt->user_data is kept for the NULL character in case of out-of-bound access.
                copy_len = MIN(evt->data_len, (MAX_HTTP_OUTPUT_BUFFER - output_len));
                if (copy_len)
                {
                    memcpy(evt->user_data + output_len, evt->data, copy_len);
                }
            }
            else
            {
                int content_len = esp_http_client_get_content_length(evt->client);
                if (output_buffer == NULL)
                {
                    // We initialize output_buffer with 0 because it is used by strlen() and similar functions therefore should be null terminated.
                    output_buffer = (char *)calloc(content_len + 1, sizeof(char));
                    output_len = 0;
                    if (output_buffer == NULL)
                    {
                        ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
                        return ESP_FAIL;
                    }
                }
                copy_len = MIN(evt->data_len, (content_len - output_len));
                if (copy_len)
                {
                    memcpy(output_buffer + output_len, evt->data, copy_len);
                }
            }
            output_len += copy_len;
        }

        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGI(TAG, "HTTP_EVENT_ON_FINISH");
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
        break;
    case HTTP_EVENT_DISCONNECTED:
    {
        ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
        int mbedtls_err = 0;
        esp_err_t err = esp_tls_get_and_clear_last_error((esp_tls_error_handle_t)evt->data, &mbedtls_err, NULL);
        if (err != 0)
        {
            ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
            ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
        }
        if (output_buffer != NULL)
        {
            free(output_buffer);
            output_buffer = NULL;
        }
        output_len = 0;
    }
    break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGI(TAG, "HTTP_EVENT_REDIRECT");
        esp_http_client_set_header(evt->client, "From", "user@example.com");
        esp_http_client_set_header(evt->client, "Accept", "text/html");
        esp_http_client_set_redirection(evt->client);
        break;
    default:
        ESP_LOGE(TAG, "Unhandled event");
        break;
    }
    return ESP_OK;
}

// static void fetch_prices_task(void *param)
esp_err_t fetch_prices_trigger(int argc, char *argv[])
{
    ESP_LOGI(TAG, "Fetch prices task has been started!");

    char *local_response_buffer = (char *)malloc(MAX_HTTP_OUTPUT_BUFFER + 1);

    // TODO make the period_from and period_to parameters dynamic
    esp_http_client_config_t config = {
        .url = "https://api.octopus.energy/v1/products/AGILE-24-10-01/electricity-tariffs/E-1R-AGILE-24-10-01-A/standard-unit-rates?period_from=2026-01-08T00:00Z&period_to=2026-01-08T23:59Z",
        .event_handler = _http_event_handler,
        .user_data = local_response_buffer,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %" PRId64,
                 esp_http_client_get_status_code(client),
                 esp_http_client_get_content_length(client));

        // Load the data!
        //
        cJSON *root = cJSON_Parse(local_response_buffer);

        if (root == NULL)
        {
            ESP_LOGE(TAG, "Failed to parse JSON");
            return ESP_FAIL;
        }

        const cJSON *countJSON = cJSON_GetObjectItemCaseSensitive(root, "count");
        ESP_LOGI(TAG, "Count: %d", countJSON->valueint);

        cJSON *resultsJSON = cJSON_GetObjectItemCaseSensitive(root, "results");

        struct tm ts;
        cJSON *resultJSON = NULL;

        auto &dayEntriesAttr = commodity_tariff_delegate.GetDayEntries();
        auto &tariffComponentsAttr = commodity_tariff_delegate.GetTariffComponents();

        cJSON_ArrayForEach(resultJSON, resultsJSON)
        {
            cJSON *valueIncVatJSON = cJSON_GetObjectItemCaseSensitive(resultJSON, "value_inc_vat");
            ESP_LOGI(TAG, "price: %d", valueIncVatJSON->valueint);

            cJSON *validFromJSON = cJSON_GetObjectItemCaseSensitive(resultJSON, "valid_from");
            ESP_LOGI(TAG, "validFrom: %s", validFromJSON->valuestring);

            strptime(validFromJSON->valuestring, "%Y-%m-%dT%H:%M:%SZ", &ts);
            ESP_LOGI(TAG, "validFrom hour: %d", ts.tm_hour);
            ESP_LOGI(TAG, "validFrom minute: %d", ts.tm_min);

            // Convert our timestamp into something we can match again
            uint16_t startTime = (ts.tm_hour * 60) + ts.tm_min;

            for (int i = 0; i < dayEntriesAttr.Value().size(); i++)
            {
                if (dayEntriesAttr.Value()[i].startTime == startTime)
                {
                    ESP_LOGI(TAG, "Updating price at %d", i);
                    tariffComponentsAttr.Value()[i].price.Value().Value().price = MakeOptional(valueIncVatJSON->valueint);
                    break;
                }
            }
        }

        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
        return ESP_FAIL;
    }
}

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

void start_matter()
{
    node::config_t node_config;

    node_t *node = node::create(&node_config, NULL, NULL);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

    esp_matter::endpoint::electrical_energy_tariff::config_t electrical_energy_tariff_config;
    endpoint_t *endpoint = esp_matter::endpoint::electrical_energy_tariff::create(node, &electrical_energy_tariff_config, ENDPOINT_FLAG_NONE, NULL);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create endpoint"));

    esp_matter::cluster::commodity_tariff::config_t commodity_tariff_config;
    commodity_tariff_config.delegate = &commodity_tariff_delegate;
    commodity_tariff_config.feature_flags = esp_matter::cluster::commodity_tariff::feature::pricing::get_id();
    cluster_t *cluster = esp_matter::cluster::commodity_tariff::create(endpoint, &commodity_tariff_config, CLUSTER_FLAG_SERVER);

    ABORT_APP_ON_FAILURE(cluster != nullptr, ESP_LOGE(TAG, "Failed to create commodity_tariff cluster"));

    esp_err_t err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));
}

static esp_matter::console::engine tariff_console;

esp_err_t print_description(const esp_matter::console::command_t *command, void *arg)
{
    ESP_LOGI(TAG, "\t%s: %s", command->name, command->description);
    return ESP_OK;
}

static esp_err_t tariff_dispatch(int argc, char *argv[])
{
    if (argc <= 0)
    {
        tariff_console.for_each_command(print_description, NULL);
        return ESP_OK;
    }

    return tariff_console.exec_command(argc, argv);
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_LOGE(TAG, "nvs_flash_init error");
    }
    ESP_ERROR_CHECK(err);

    start_matter();

#if CONFIG_ENABLE_CHIP_SHELL

    ESP_LOGI(TAG,"Enabling CHIP SHELL");

    static const esp_matter::console::command_t tariff_command = {
        .name = "tariff",
        .description = "Tariff commands. Usage: matter esp tariff <tariff_command>.",
        .handler = tariff_dispatch,
    };

    static const esp_matter::console::command_t tariff_commands[] = {
        {
            .name = "fetch",
            .description = "Fetches the Octopus Agile prices.",
            .handler = fetch_prices_trigger,
        }};

    tariff_console.register_commands(tariff_commands, sizeof(tariff_commands) / sizeof(esp_matter::console::command_t));

    esp_matter::console::add_commands(&tariff_command, 1);

    esp_matter::console::init();
#endif
}
