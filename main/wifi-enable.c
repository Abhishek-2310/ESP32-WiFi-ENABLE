#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"
/** DEFINES **/
#define WIFI_SUCCESS 1 << 0
#define WIFI_FAILURE 1 << 1
#define TCP_SUCCESS 1 << 0
#define TCP_FAILURE 1 << 1
#define MAX_FAILURES 10
#define PORT 80
#define IP_ADDRESS "10.0.0.119"
#define BUFFER_SIZE 1024
#define BLINK_LED 2

/** GLOBALS **/

// event group to contain status information
static EventGroupHandle_t wifi_event_group;
// retry tracker
static int s_retry_num = 0;

// task tag
static const char *TAG = "WIFI";

//HTTP message
// const char* display_msg = "HTTP/1.1 200 OK\r\n"
//                     "Server: ESP32 Controlled Machine\r\n"
//                     "Content-Type: text/html\r\n"
//                     "Content-Length: 250\r\n"
//                     "Connection: Closed\r\n"
//                     "\r\n"
//                     "<!DOCTYPE html>\r\n"
//                     "<html>\r\n"
//                     "<head>\r\n"
//                     "<title>My ESP32</title>\r\n"
//                     "</head>\r\n"
//                     "<body>\r\n"
//                     "<h1>Welcome to ESP32!</h1>\r\n"
//                     "<a href='/TURN_ON_LED'>TURN ON!</a>\r\n"
//                     "<a href='/TURN_OFF_LED'>TURN OFF!</a>\r\n"
//                     "</body>\r\n"
//                     "</html>\r\n";

const char* on_msg = "HTTP/1.1 200 OK\r\n"
                    "Server: ESP32 Controlled Machine\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 250\r\n"
                    "Connection: Closed\r\n"
                    "\r\n"
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "<head>\r\n"
                    "<title>My ESP32</title>\r\n"
                    "</head>\r\n"
                    "<body>\r\n"
                    "<h1>Welcome to ESP32!</h1>\r\n"
                    "<h1>GPIO Turned ON!</h1>\r\n"
                    "<img src = 'https://upload.wikimedia.org/wikipedia/commons/3/3b/Check_Green.png'>\r\n"
                    "<a href='/TURN_OFF_LED'>TURN OFF!</a>\r\n"
                    "</body>\r\n"
                    "</html>\r\n";

const char* off_msg = "HTTP/1.1 200 OK\r\n"
                    "Server: ESP32 Controlled Machine\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: 290\r\n"
                    "Connection: Closed\r\n"
                    "\r\n"
                    "<!DOCTYPE html>\r\n"
                    "<html>\r\n"
                    "<head>\r\n"
                    "<title>My ESP32</title>\r\n"
                    "</head>\r\n"
                    "<body>\r\n"
                    "<h1>Welcome to ESP32!</h1>\r\n"
                    "<h1>GPIO Turned OFF!</h1>\r\n"
                    "<img src = 'https://upload.wikimedia.org/wikipedia/commons/8/83/Eo_circle_red_white_letter-x.svg' width='76' height='72'>\r\n"
                    "<a href='/TURN_ON_LED'>TURN ON!</a>\r\n"
                    "</body>\r\n"
                    "</html>\r\n";

// Flags
uint8_t rFlag = 0;

/** FUNCTIONS **/

//event handler for wifi events
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
	{
		ESP_LOGI(TAG, "Connecting to AP...");
		esp_wifi_connect();
	} else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
	{
		if (s_retry_num < MAX_FAILURES)
		{
			ESP_LOGI(TAG, "Reconnecting to AP...");
			esp_wifi_connect();
			s_retry_num++;
		} else {
			xEventGroupSetBits(wifi_event_group, WIFI_FAILURE);
		}
	}
}

//event handler for ip events
static void ip_event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
	if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
	{
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "STA IP: " IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(wifi_event_group, WIFI_SUCCESS);
    }
}

// connect to wifi and return the result
esp_err_t connect_wifi()
{
	int status = WIFI_FAILURE;

	/** INITIALIZE ALL THE THINGS **/
	//initialize the esp network interface
	ESP_ERROR_CHECK(esp_netif_init());

	//initialize default esp event loop
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	//create wifi station in the wifi driver
	esp_netif_create_default_wifi_sta();

	//setup wifi station with the default wifi configuration
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    /** EVENT LOOP CRAZINESS **/
	wifi_event_group = xEventGroupCreate();

    esp_event_handler_instance_t wifi_handler_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        &wifi_handler_event_instance));

    esp_event_handler_instance_t got_ip_event_instance;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &ip_event_handler,
                                                        NULL,
                                                        &got_ip_event_instance));

    /** START THE WIFI DRIVER **/
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    
    // set the wifi controller to be a station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );

    // set the wifi config
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );

    // start the wifi driver
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "STA initialization complete");

    /** NOW WE WAIT **/
    EventBits_t bits = xEventGroupWaitBits(wifi_event_group,
            WIFI_SUCCESS | WIFI_FAILURE,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_SUCCESS) {
        ESP_LOGI(TAG, "Connected to ap");
        status = WIFI_SUCCESS;
    } else if (bits & WIFI_FAILURE) {
        ESP_LOGI(TAG, "Failed to connect to ap");
        status = WIFI_FAILURE;
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
        status = WIFI_FAILURE;
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, got_ip_event_instance));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_handler_event_instance));
    vEventGroupDelete(wifi_event_group);

    return status;
}

// connect to the server and return the result
esp_err_t connect_tcp_server(void)
{
	int server_socket, client_socket;
    struct sockaddr_in server_address, client_address;
    char buffer[BUFFER_SIZE];
    // char *response = "Hello from ESP32!\n";

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    server_address.sin_port = htons(PORT);

    // Bind socket to address and port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    // Listen for connections
    if (listen(server_socket, 1) == -1) {
        perror("Error listening for connections");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d...\n", PORT);

    while (1) {
        // Accept connection
        socklen_t client_address_len = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr *)&client_address, &client_address_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            exit(EXIT_FAILURE);
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

        // Send response to the client
        // if (send(client_socket, response, strlen(response), 0) == -1) {
        //     perror("Error sending response to the client");
        //     exit(EXIT_FAILURE);
        // }

        // bzero(buffer, sizeof(buffer));
        int r = -1;

        r = read(client_socket, buffer, sizeof(buffer)-1);
        if(r > 0) 
        {
            printf("message: %s\n",buffer);
            r = -1;

            if(strstr(buffer, "TURN_ON_LED"))
            {
                write(client_socket, on_msg, strlen(on_msg));
                gpio_set_level(BLINK_LED, 1);
            }
            else
            {
                write(client_socket, off_msg, strlen(off_msg));
                gpio_set_level(BLINK_LED, 0);
            }

        }
        // Close the connection
        close(client_socket);
    }

    // Close the server socket
    close(server_socket);

    return TCP_SUCCESS;
}


void app_main(void)
{
    esp_err_t status = WIFI_FAILURE;

    gpio_reset_pin(BLINK_LED);
    gpio_set_direction(BLINK_LED, GPIO_MODE_OUTPUT);   
	//initialize storage
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // connect to wireless AP
	status = connect_wifi();
	if (WIFI_SUCCESS != status)
	{
		ESP_LOGI(TAG, "Failed to associate to AP, dying...");
		return;
	}
	
	status = connect_tcp_server();
	if (TCP_SUCCESS != status)
	{
		ESP_LOGI(TAG, "Failed to connect to remote server, dying...");
		return;
	}

}
