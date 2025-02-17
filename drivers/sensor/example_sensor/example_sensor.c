/*
 * Copyright (c) 2021 Nordic Semiconductor ASA
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT zephyr_example_sensor

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(example_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct example_sensor_data
{
	int state;
	FILE *csv_file;
};

static int example_sensor_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	struct example_sensor_data *data = dev->data;
	int value;
	int ret = fscanf(data->csv_file, "%d", &value);

	if (!data->csv_file)
	{
		LOG_ERR("CSV file is not open");
		return -ENOENT;
	}

		if (ret == 1)
	{
		data->state = value;
	}
	else if (ret == EOF)
	{
		LOG_INF("CSV file ended, stopping sensor updates");
		fclose(data->csv_file);
		data->csv_file = NULL;
		data->state = 0;
		return -ENODATA;
	}
	else
	{
		LOG_ERR("Malformed CSV row");
		return -EINVAL;
	}

	return 0;
}

static int example_sensor_channel_get(const struct device *dev,
									  enum sensor_channel chan,
									  struct sensor_value *val)
{
	struct example_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_AMBIENT_TEMP)
	{
		return -ENOTSUP;
	}

	if (data->state == 0)
	{
		return -ENODATA;
	}

	val->val1 = data->state;
	val->val2 = 0;

	return 0;
}

static DEVICE_API(sensor, example_sensor_api) = {
	.sample_fetch = &example_sensor_sample_fetch,
	.channel_get = &example_sensor_channel_get,
};

static int example_sensor_init(const struct device *dev)
{
	struct example_sensor_data *data = dev->data;

	if (!data->csv_file)
	{
		data->csv_file = fopen(CONFIG_EXAMPLE_SENSOR_CSV_FILE_PATH, "r");

		if (!data->csv_file)
		{
			LOG_ERR("Failed to open CSV file");
			return -ENOENT;
		}
	}

	return 0;
}

#define EXAMPLE_SENSOR_INIT(i)                                 \
	static struct example_sensor_data example_sensor_data_##i; \
                                                               \
	DEVICE_DT_INST_DEFINE(i, example_sensor_init, NULL,        \
						  &example_sensor_data_##i,            \
						  NULL, POST_KERNEL,                   \
						  CONFIG_SENSOR_INIT_PRIORITY, &example_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(EXAMPLE_SENSOR_INIT)
