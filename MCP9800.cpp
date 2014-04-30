#include "MCP9800.h"

bool MCP9800::init()
{
	Wire.begin();
	uint8_t config;
	read(REG_CONFIG, &config, 1);
	bool ok = (config == 0); // all values are 0 on power up
	return ok;
}

void MCP9800::write(byte reg, byte *data, int8_t len)
{
	Wire.beginTransmission(MCP9800_ADDRESS);
	Wire.write(reg);

	while (len--)
	{
		Wire.write(*data++);
	}

	Wire.endTransmission();
}

void MCP9800::read(byte reg, byte *buffer, uint8_t numBytes)
{
	Wire.beginTransmission(MCP9800_ADDRESS);
	Wire.write(reg);
	Wire.endTransmission();
	int8_t bytes = Wire.requestFrom((uint8_t)MCP9800_ADDRESS, numBytes);
	if (buffer != NULL)
	{
		while (bytes--)
		{
			*buffer++ = Wire.read();
		}
	}
}

void MCP9800::setOneShot(bool enabled)
{
	// device needs to be in shutdown mode first
	setShutdown(enabled);

	uint8_t config;
	read(REG_CONFIG, &config, 1);
	config &= ~(1 << CONFIG_ONE_SHOT);
	config |= (enabled << CONFIG_ONE_SHOT);
	write(REG_CONFIG, &config, 1);
}

void MCP9800::setResolution(mcp9800_adc_resolution_t resolution)
{
	uint8_t config;
	read(REG_CONFIG, &config, 1);
	config &= ~(3 << CONFIG_ADC_RES);
	config |= (resolution << CONFIG_ADC_RES);
	write(REG_CONFIG, &config, 1);
	adc = resolution;
}

void MCP9800::setFaultQueue(mcp9800_fault_queue_t numFaults)
{
	uint8_t config;
	read(REG_CONFIG, &config, 1);
	config &= ~(3 << CONFIG_FAULT_QUEUE);
	config |= (numFaults << CONFIG_FAULT_QUEUE);
	write(REG_CONFIG, &config, 1);
}

void MCP9800::setShutdown(bool shutdown)
{
	uint8_t config;
	read(REG_CONFIG, &config, 1);
	config &= ~(1 << CONFIG_SHUTDOWN);
	config |= (shutdown << CONFIG_SHUTDOWN);
	write(REG_CONFIG, &config, 1);
}

void MCP9800::setAlertMode(mcp9800_alert_mode_t alertMode, bool polarity)
{
	uint8_t config;
	read(REG_CONFIG, &config, 1);
	config &= ~(1 << CONFIG_COMP_INT);
	config &= ~(1 << CONFIG_ALERT_POL);
	config |= (alertMode << CONFIG_COMP_INT) | (polarity << CONFIG_ALERT_POL);
	write(REG_CONFIG, &config, 1);
}

void MCP9800::setAlertLimits(int8_t lower, int8_t upper)
{
	if (lower > 127) lower = 127;
	if (lower < -127) lower = -127;
	if (upper > 127) upper = 127;
	if (upper < -127) upper = -127;
	bool negativeLower = lower < 0;
	bool negativeUpper = upper < 0;
	byte hysterisis = (negativeLower << 7) | lower;
	byte limit = (negativeUpper << 7) | upper;
	write(REG_HYSTERISIS, &hysterisis, 1);
	write(REG_LIMIT, &limit, 1);
}

void MCP9800::resetAlert()
{
	// Reading any register resets the alarm
	read(REG_CONFIG, NULL, 1);
}

int16_t MCP9800::readRawData()
{
	uint8_t temp[2];
	read(REG_TEMP, temp, 2);
	return (temp[0] << 8) | temp[1];
}

int16_t MCP9800::readCelsius()
{
	int16_t temp = readRawData();
	int8_t highByte = (temp >> 8);
	uint8_t lowByte = (temp & 0xFF);
	temp = (highByte << 4) | (lowByte >> 4);

	return temp;
}

int16_t MCP9800::readFahrenheit()
{
	return (readCelsius() * 18 + 325) / 10;
}

float MCP9800::readCelsiusf()
{
	int16_t temp = readCelsius();
	return temp / 16.0f;
}

#define DATA_TO_CELSIUS 0.1125 // 16 / 9 / 5
float MCP9800::readFahrenheitf()
{
	return readCelsius() * DATA_TO_CELSIUS + 32;
}