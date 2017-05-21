#include "IPConfigurator.h"
#include <string.h>
#include <stdio.h>

void _setQuarterValueAndShift(s_IpAddressConfigurator *configurator, uint8_t *quarterValue, uint8_t *shift);

s_IpAddressConfigurator IPConfigurator_create() {
	s_IpAddressConfigurator configurator;
	configurator.ipAddress = 0;
	configurator.currentQuarter = 4;
	return configurator;
}

char IPConfigurator_addChar(s_IpAddressConfigurator *configurator, const char c) {
	uint8_t quarterValue, newValue, shift;
	if (configurator->currentQuarter <= 0) {
		return 0;
	}

	if (c == '.') {
		configurator->currentQuarter--;
		return 1;
	}

	shift = (uint8_t) ((configurator->currentQuarter - 1) * 8);
	quarterValue = (uint8_t) (configurator->ipAddress >> shift);
	newValue = (uint8_t) (quarterValue * 10 + c);
	if (newValue < quarterValue) {
		configurator->currentQuarter--;
		return IPConfigurator_addChar(configurator, c);
	}
	configurator->ipAddress &= (unsigned) ~(255 << shift);
	configurator->ipAddress |= (unsigned) (newValue << shift);

	if (newValue > 25 || (c == 0 && !newValue)) {
		configurator->currentQuarter--;
	}

	return 1;
}

void IPConfigurator_removeChar(s_IpAddressConfigurator *configurator) {
	uint8_t quarterValue, newValue, shift;
	if (configurator->currentQuarter == 4 && configurator->ipAddress == 0) {
		return;
	}

	_setQuarterValueAndShift(configurator, &quarterValue, &shift);

	if (quarterValue == 0 || !configurator->currentQuarter) {
		configurator->currentQuarter++;
		_setQuarterValueAndShift(configurator, &quarterValue, &shift);
	}

	newValue = quarterValue / 10;
	configurator->ipAddress &= (unsigned) ~(255 << shift);
	configurator->ipAddress |= (unsigned) (newValue << shift);
}

void _setQuarterValueAndShift(s_IpAddressConfigurator *configurator, uint8_t *quarterValue, uint8_t *shift) {
	uint8_t quarter;
	quarter = configurator->currentQuarter == 0 ? 1 : configurator->currentQuarter;
	*shift = (uint8_t) ((quarter - 1) * 8);
	*quarterValue = (uint8_t) (configurator->ipAddress >> *shift);
}

void IPConfigurator_toString(s_IpAddressConfigurator *configurator, char *ip, char full) {
	if (full) {
		snprintf(
			ip, 16,
			"%d.%d.%d.%d",
			255 & (configurator->ipAddress >> 24),
			255 & (configurator->ipAddress >> 16),
			255 & (configurator->ipAddress >> 8),
			255 & configurator->ipAddress
		);
		return;
	}

	int quarter = 4;
	uint8_t shift, value;
	char buff[4];

	ip[0] = '\0';
	while (quarter && quarter >= configurator->currentQuarter) {
		shift = (uint8_t) ((quarter - 1) * 8);
		value = (uint8_t) (configurator->ipAddress >> shift);
		sprintf(buff, "%d", value);
		if (value || quarter > configurator->currentQuarter) {
			strncat(ip, buff, 3);
		}
		if (quarter > 1 && quarter > configurator->currentQuarter) {
			strncat(ip, ".", 1);
		}
		--quarter;
	}

	if (configurator->currentQuarter > 0) {
		strcat(ip, "_");
	}
}
