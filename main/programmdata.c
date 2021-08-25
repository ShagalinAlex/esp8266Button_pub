/*
 * programmdata.c
 *
 *  Created on: 21 џэт. 2020 у.
 *      Author: Nemo
 */

#include <stdio.h>
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_spiffs.h"
#include "programmdata.h"

int ProgDataRead(FlashDataParam *params) {
	printf("Initializing SPIFFS for read\n\r");

	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 3,
			.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			printf("Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			printf("Failed to find SPIFFS partition");
		} else {
			printf("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}

	// Use POSIX and C standard library functions to work with files.
	// First create a file.
	printf("Opening file");
	FILE* f = fopen("/spiffs/configuration.dat", "r");
	if (f == NULL) {
		printf("Failed to open file for reading");
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}
	fread(params, sizeof(FlashDataParam), 1, f);
	fclose(f);
	printf("SPIFFS unmounted");
	esp_vfs_spiffs_unregister(NULL);
	return 0;
}

int ProgDataWrite(FlashDataParam *params) {
	printf("Initializing SPIFFS for write");

	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 3,
			.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			printf("Failed to mount or format filesystem\n\r");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			printf("Failed to find SPIFFS partition\n\r");
		} else {
			printf("Failed to initialize SPIFFS (%s)\n\r", esp_err_to_name(ret));
		}
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}

	printf("Opening file");
	FILE* f = fopen("/spiffs/configuration.dat", "w");
	if (f == NULL) {
		printf("Failed to open file for writing\n\r");
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}
	fwrite(params, sizeof(FlashDataParam), 1, f);
	fclose(f);

	esp_vfs_spiffs_unregister(NULL);
	printf("SPIFFS unmounted\n\r");
	return 0;
}

int ProgMQTTDataRead(MQTTDataParam *params) {
	printf("Initializing SPIFFS for read\n\r");

	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 3,
			.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			printf("Failed to mount or format filesystem");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			printf("Failed to find SPIFFS partition");
		} else {
			printf("Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
		}
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}

	// Use POSIX and C standard library functions to work with files.
	// First create a file.
	printf("Opening file");
	FILE* f = fopen("/spiffs/MQTT.dat", "r");
	if (f == NULL) {
		printf("Failed to open file for reading");
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}
	fread(params, sizeof(MQTTDataParam), 1, f);
	fclose(f);
	printf("SPIFFS unmounted");
	esp_vfs_spiffs_unregister(NULL);
	return 0;
}

int ProgMQTTDataWrite(MQTTDataParam *params) {
	printf("Initializing SPIFFS for write");

	esp_vfs_spiffs_conf_t conf = {
			.base_path = "/spiffs",
			.partition_label = NULL,
			.max_files = 3,
			.format_if_mount_failed = true
	};

	// Use settings defined above to initialize and mount SPIFFS filesystem.
	// Note: esp_vfs_spiffs_register is an all-in-one convenience function.
	esp_err_t ret = esp_vfs_spiffs_register(&conf);

	if (ret != ESP_OK) {
		if (ret == ESP_FAIL) {
			printf("Failed to mount or format filesystem\n\r");
		} else if (ret == ESP_ERR_NOT_FOUND) {
			printf("Failed to find SPIFFS partition\n\r");
		} else {
			printf("Failed to initialize SPIFFS (%s)\n\r", esp_err_to_name(ret));
		}
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}

	printf("Opening file");
	FILE* f = fopen("/spiffs/MQTT.dat", "w");
	if (f == NULL) {
		printf("Failed to open file for writing\n\r");
		esp_vfs_spiffs_unregister(NULL);
		return -1;
	}
	fwrite(params, sizeof(MQTTDataParam), 1, f);
	fclose(f);

	esp_vfs_spiffs_unregister(NULL);
	printf("SPIFFS unmounted\n\r");
	return 0;
}
