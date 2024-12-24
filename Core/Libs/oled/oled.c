/**
 *	TechMaker
 *	https://techmaker.ua
 *
 *	STM32 LCD OLED Library for 0.96" / 1.3" displays using I2C/SPI
 *	based on library by Tilen Majerle, Adafruit GFX & Adafruit TFT LCD libraries
 *	16 Aug 2018 by Alexander Olenyev <sasha@techmaker.ua>
 *
 *	Changelog:
 *		- v1.2 added SPI interface & return status codes
 *		- v1.1 added Courier New font family with Cyrillic (CP1251), created using TheDotFactory font generator
 *		- v1.0 added support for SSD1306 and SH1106 chips
 */

/**
 * |----------------------------------------------------------------------
 * | Copyright (C) Tilen Majerle, 2015
 * |
 * | This program is free software: you can redistribute it and/or modify
 * | it under the terms of the GNU General Public License as published by
 * | the Free Software Foundation, either version 3 of the License, or
 * | any later version.
 * |
 * | This program is distributed in the hope that it will be useful,
 * | but WITHOUT ANY WARRANTY; without even the implied warranty of
 * | MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * | GNU General Public License for more details.
 * |
 * | You should have received a copy of the GNU General Public License
 * | along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * |----------------------------------------------------------------------
 */

#include "oled.h"

static int16_t m_width;
static int16_t m_height;
static int16_t m_cursor_x;
static int16_t m_cursor_y;

static uint16_t m_textcolor;
static uint8_t m_font;
#if defined(OLED_COLORZONE)
static OLED_Zone_t m_inverted;
#else
static uint8_t m_inverted;
#endif
static uint8_t m_wrap;

static font_t * fonts[] = {
#ifdef USE_FONT8
					&Font8,
#endif
#ifdef USE_FONT12
					&Font12,
#endif
#ifdef USE_FONT16
					&Font16,
#endif
#ifdef USE_FONT20
					&Font20,
#endif
#ifdef USE_FONT24
					&Font24,
#endif
					};
const static uint8_t fontsNum = sizeof(fonts) / sizeof(fonts[0]);

/* OLED data buffer */
static uint8_t OLED_Buffer[OLEDWIDTH * OLEDHEIGHT / 8];

#if defined(OLED_I2C)
/* OLED I2C Handler */
static I2C_HandleTypeDef * OLED_hi2c;
#elif defined(OLED_SPI)
/* OLED SPI Handler */
static SPI_HandleTypeDef * OLED_hspi;
#endif

#if defined(SSD1306)
static const uint8_t SSD1306_regValues[] = {
		SSD1306_SETDISPLAY_OFF, 0,							// Display off
#if defined (USE_HORZMODE)
		SSD1306_SETMEMORYMODE, 1, SSD1306_MEMORYMODE_HORZ,	// Set Memory Addressing Mode to Horizontal
		SSD1306_SETCOLUMNADDR, 2, 0x00, OLEDWIDTH - 1,		// Set column address window from 0 to 127
		SSD1306_SETPAGEADDR, 2, 0x00, 0x07,					// Set page address window from 0 to 7
#elif defined (USE_PAGEMODE)
		SSD1306_SETCOLUMNSTARTLOW | 0x00, 0,				// Set column start address to 0 (lower nibble)
		SSD1306_SETCOLUMNSTARTHIGH | 0x00, 0,				// Set column start address to 0 (higher nibble)
		SSD1306_SETPAGESTART | 0x00, 0,						// Set page start address to 0 (use | 0x07 for 7th page)
#endif
		SSD1306_SETDISPLAYSTARTLINE | 0x00, 0,				// Set page start line to 0 (use | 0xFF for 63rd line)
		SSD1306_SETSEGREMAP_127, 0,							// Set segment re-map from 0 to 127
		SSD1306_SETDISPLAY_NORMAL, 0,						// Set normal display (not inverted)
		SSD1306_SETMUXRATIO, 1, OLEDHEIGHT - 1,				// Set multiplex ratio (default 63)
		SSD1306_SETCOMSCAN_DEC, 0,							// Set COM Output Scan Direction to Reverse
		SSD1306_SETDISPLAYOFFSET, 1, 0x00,					// Set display offset to 0
#if OLEDHEIGHT <= 32
		SSD1306_SETCOMPINS, 1, SSD1306_COMPINS_SEQUENTIAL,	// Set COM pins hardware configuration: normal configuration / no left-right remap
#else
		SSD1306_SETCOMPINS, 1, SSD1306_COMPINS_ALTERNATIVE,	// Set COM pins hardware configuration: alternative configuration / no left-right remap
#endif
		SSD1306_SETDISPLAYCLOCK, 1, SSD1306_OSCFREQ_534KHZ | SSD1306_CLOCKDIV_1,	// Set oscillator frequency to max, display clock divide ratio to 1 (1 DCLK = CLK)
		SSD1306_SETPRECHARGEPERIOD, 1, 0x22,				// Set pre-charge period: phase 1 period to 2DCLK, phase 2 period to 2DCLK
		SSD1306_SETCONTRAST, 1, 0xFF,						// Set contrast control register to 256
		SSD1306_SCROLL_DEACTIVATE, 0,						// Disable scrolling
		SSD1306_SETFADEOUTANDBLINK, 1, SSD1306_FADE_OFF,	// Disable fading
		SSD1306_SETZOOMIN, 1, SSD1306_ZOOMIN_OFF,			// Disable zoom-in
		SSD1306_SETVCOMLEVEL, 1, SSD1306_VCOM_083VCC,		// Set Vcomh deselect at 0.83*Vcc
		SSD1306_DISPLAY_RAM, 0,								// Set output to follow RAM content
		SSD1306_SETCHARGEPUMP, 1, SSD1306_CHARGEPUMP_ON,	// Enable charge pump
		SSD1306_SETDISPLAY_ON, 0							// Display on
};
#elif defined(SH1106)
static const uint8_t SH1106_regValues[] = {
		SH1106_SETDISPLAY_OFF, 0,							// Display off
		// SH1106 ram has 132px, OLED matrix is 128px, they are center aligned - so image has to be shifted 2px!!
		SH1106_SETCOLUMNSTARTLOW | 0x02, 0,					// Set low column address to 2
		SH1106_SETCOLUMNSTARTHIGH | 0x00, 0,				// Set high column address to 0
		SH1106_SETPAGESTART | 0x00, 0,						// Set Page Start Address to 0 (use | 0x07 for 7th Page)
		SH1106_SETDISPLAYSTARTLINE | 0x00, 0,				// Set page start line to 0 (use | 0xFF for 63rd line)
		SH1106_SETSEGREMAP_127, 0,							// Set segment re-map from 0 to 127
		SH1106_SETDISPLAY_NORMAL, 0,						// Set normal display (not inverted)
		SH1106_SETMUXRATIO, 1, OLEDHEIGHT - 1,				// Set multiplex ratio (default 63)
		SH1106_SETCOMSCAN_DEC, 0,							// Set COM Output Scan Direction to Reverse
		SH1106_SETDISPLAYOFFSET, 1, 0x00,					// Set display offset to 0
#if OLEDHEIGHT <= 32
		SH1106_SETCOMPINS, 1, SH1106_COMPINS_SEQUENTIAL,	// Set COM pins hardware configuration: normal configuration / no left-right remap
#else
		SH1106_SETCOMPINS, 1, SH1106_COMPINS_ALTERNATIVE,	// Set COM pins hardware configuration: alternative configuration / no left-right remap
#endif
		SH1106_SETDISPLAYCLOCK, 1, SH1106_OSCFREQ_534KHZ | SH1106_CLOCKDIV_1,	// Set display clock divide ratio to 8 (8 DCLK = CLK), oscillator frequency to 260Hz
		SH1106_SETPRECHARGEPERIOD, 1, 0x22,					// Set pre-charge period: phase 1 period to 2DCLK, phase 2 period to 2DCLK
		SH1106_SETCONTRAST, 1, 0x80,						// Set contrast control register to 256
		SH1106_DISPLAY_RAM, 0,								// Set output to follow RAM content
		SH1106_SETVCOMLEVEL, 1, SH1106_SETVCOM_100V,		// Set Vcomh level to 0.83xVCC
		SH1106_SETPUMPVOLTAGE, 1, SH1106_PUMPVOLTAGE_9V0,	// Set charge pump voltage to 9.0V
		SH1106_SETDCDC, 1, SH1106_DCDC_ON,					// Enable charge pump
		SH1106_SETDISPLAY_ON, 0								// Display on
};
#endif

#if defined(OLED_I2C)
/* ==================================================
 * ============= I2C Specific functions =============
 * ================================================== */

inline static HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd) {
	return HAL_I2C_Mem_Write(OLED_hi2c, OLED_I2C_ADDR << 1, I2C_CONTROL_CMD, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 10);
}

inline static HAL_StatusTypeDef OLED_WriteData(uint8_t * data, uint32_t len) {
	return HAL_I2C_Mem_Write(OLED_hi2c, OLED_I2C_ADDR << 1, I2C_CONTROL_DATA, I2C_MEMADD_SIZE_8BIT, data, len, 100);
}

HAL_StatusTypeDef OLED_Init(I2C_HandleTypeDef * hi2c) {
	uint32_t status = HAL_OK;
	m_width = OLEDWIDTH;
	m_height = OLEDHEIGHT;
	m_cursor_y = m_cursor_x = 0;
	m_font = 0;
	m_textcolor = White;
	m_inverted = 0;
	m_wrap = 1;
	OLED_hi2c = hi2c;
	HAL_Delay(50);
	/* Check connection, try 10 times */
	uint32_t errcnt = 0;
	while (HAL_I2C_IsDeviceReady(OLED_hi2c, OLED_I2C_ADDR << 1, 5, 200) != HAL_OK) {
		if (errcnt++ > 10) {
			return HAL_ERROR;
		}
		HAL_Delay(50);
	}
	/* A little delay */
	HAL_Delay(50);
	/* Set register values */
#if defined(SSD1306)
	uint8_t i = 0;
	while (i < sizeof(SSD1306_regValues) / sizeof(SSD1306_regValues[0])) {
		uint8_t r = SSD1306_regValues[i++];
		uint8_t len = SSD1306_regValues[i++];
		status += OLED_WriteCmd(r);
		for (uint8_t d = 0; d < len; d++) {
			uint8_t x = SSD1306_regValues[i++];
			status += OLED_WriteCmd(x);
		}
	}
#elif defined(SH1106)
	uint8_t i = 0;
	while (i < sizeof(SH1106_regValues) / sizeof(SH1106_regValues[0])) {
		uint8_t r = SH1106_regValues[i++];
		uint8_t len = SH1106_regValues[i++];
		status += OLED_WriteCmd(r);
		for (uint8_t d = 0; d < len; d++) {
			uint8_t x = SH1106_regValues[i++];
			status += OLED_WriteCmd(x);
		}
	}
#endif
	/* Clear screen */
	OLED_FillScreen(Black);
	/* Update screen */
	status += OLED_UpdateScreen();
	OLED_SetTextSize(0);
	OLED_SetTextColor(White);
	return (status == HAL_OK) ? HAL_OK : HAL_ERROR;
}

/* ==================================================
 * ========= End of I2C Specific functions ==========
 * ================================================== */

#elif defined(OLED_SPI)
/*
 * ==================================================
 * ============= SPI Specific functions =============
 * ================================================== */

inline static HAL_StatusTypeDef OLED_WriteCmd(uint8_t cmd) {
	OLED_CD_COMMAND();
	return HAL_SPI_Transmit(OLED_hspi, &cmd, 1, 10);
}

inline static HAL_StatusTypeDef OLED_WriteData(uint8_t * data, uint32_t len) {
	OLED_CD_DATA();
	return HAL_SPI_Transmit(OLED_hspi, data, len, 100);
}
HAL_StatusTypeDef OLED_Init(SPI_HandleTypeDef * hspi) {
	uint32_t status = HAL_OK;
	m_width = OLEDWIDTH;
	m_height = OLEDHEIGHT;
	m_cursor_y = m_cursor_x = 0;
	m_font = 0;
	m_textcolor = White;
	m_inverted = 0;
	m_wrap = 1;
	OLED_hspi = hspi;
	OLED_RST_ACTIVE();
	HAL_Delay(50);
	OLED_RST_IDLE();
	HAL_Delay(50);
	/* Set register values */
#if defined(SSD1306)
	uint8_t i = 0;
	while (i < sizeof(SSD1306_regValues) / sizeof(SSD1306_regValues[0])) {
		uint8_t r = SSD1306_regValues[i++];
		uint8_t len = SSD1306_regValues[i++];
		status += OLED_WriteCmd(r);
		for (uint8_t d = 0; d < len; d++) {
			uint8_t x = SSD1306_regValues[i++];
			status += OLED_WriteCmd(x);
		}
	}
#elif defined(SH1106)
	uint8_t i = 0;
	while (i < sizeof(SH1106_regValues) / sizeof(SH1106_regValues[0])) {
		uint8_t r = SH1106_regValues[i++];
		uint8_t len = SH1106_regValues[i++];
		status += OLED_WriteCmd(r);
		for (uint8_t d = 0; d < len; d++) {
			uint8_t x = SH1106_regValues[i++];
			status += OLED_WriteCmd(x);
		}
	}
#endif
	/* Clear screen */
	OLED_FillScreen(Black);
	/* Update screen */
	status += OLED_UpdateScreen();
	OLED_SetTextSize(0);
	OLED_SetTextColor(White);
	return (status == HAL_OK) ? HAL_OK : HAL_ERROR;
}
/* ==================================================
 * ========= End of SPI Specific functions ==========
 * ================================================== */
#endif


/* ==================================================
 * ======== Interface independent functions =========
 * ================================================== */

HAL_StatusTypeDef OLED_UpdateScreen(void) {
#if defined (SSD1306)
#if defined(USE_HORZMODE)
	return OLED_WriteData(OLED_Buffer, OLEDWIDTH * OLEDHEIGHT / 8);
#elif defined(USE_PAGEMODE)
	uint32_t status = HAL_OK;
	for (uint8_t i = 0; i < 8; i++) {
		status += OLED_WriteCmd(SSD1306_SETCOLUMNSTARTLOW | 0x00);
		status += OLED_WriteCmd(SSD1306_SETCOLUMNSTARTHIGH | 0x00);
		status += OLED_WriteCmd(SSD1306_SETPAGESTART | i);
		status += OLED_WriteData(&OLED_Buffer[OLEDWIDTH * i], OLEDWIDTH);
	}
	return (status == HAL_OK) ? HAL_OK : HAL_ERROR;
#endif
#elif defined (SH1106)
	uint32_t status = HAL_OK;
	for (uint8_t i = 0; i < 8; i++) {
		// SH1106 ram has 132px, OLED matrix is 128px, they are center aligned - so image has to be shifted 2px!!
		status += OLED_WriteCmd(SH1106_SETCOLUMNSTARTLOW | 0x02);
		status += OLED_WriteCmd(SH1106_SETCOLUMNSTARTHIGH | 0x00);
		status += OLED_WriteCmd(SH1106_SETPAGESTART | i);
		status += OLED_WriteData(&OLED_Buffer[OLEDWIDTH * i], OLEDWIDTH);
	}
	return (status == HAL_OK) ? HAL_OK : HAL_ERROR;
#endif
}

void OLED_DrawPixel(uint8_t x, uint8_t y, OLED_Color_t color) {
	if (x >= OLEDWIDTH || y >= OLEDHEIGHT) {
		return;	// Error
	}
	/* Check if pixels are inverted */
#if defined(OLED_COLORZONE)
	switch(m_inverted) {
	case Zone_None:
		break;
	case Zone_Color1:
		if (x * y < OLED_COLORZONE_EDGE) {
			color = (OLED_Color_t) !color;
		}
		break;
	case Zone_Color2:
		if (x * y >= OLED_COLORZONE_EDGE) {
			color = (OLED_Color_t) !color;
		}
		break;
	case Zone_Both:
		color = (OLED_Color_t) !color;
		break;
	}
#else
	if (m_inverted) {
		color = (OLED_Color_t) !color;
	}
#endif

	/* Set color */
	if (color == White) {
		OLED_Buffer[x + (y / 8) * OLEDWIDTH] |= 1 << (y % 8);
	} else {
		OLED_Buffer[x + (y / 8) * OLEDWIDTH] &= ~(1 << (y % 8));
	}
}

void OLED_FillScreen(OLED_Color_t color) {
	/* Set memory */
	memset(OLED_Buffer, (color == Black) ? 0x00 : 0xFF, OLEDWIDTH * OLEDHEIGHT / 8);
}

#if defined(OLED_COLORZONE)
void OLED_ToggleInvert(OLED_Zone_t zone) {
	uint16_t start = 0, end = 0;
	/* Toggle invert */
	m_inverted ^= zone;
	/* Do memory toggle */
	switch(zone) {
	case Zone_None:
		return;
	case Zone_Color1:
		start = 0;
		end = OLED_COLORZONE_EDGE;
		break;
	case Zone_Color2:
		start = OLED_COLORZONE_EDGE;
		end = sizeof(OLED_Buffer);
		break;
	case Zone_Both:
		start = 0;
		end = sizeof(OLED_Buffer);
		break;
	}
	for (uint16_t i = start; i < end; i++) {
		OLED_Buffer[i] = ~OLED_Buffer[i];
	}
}
#else
void OLED_ToggleInvert(void) {
	/* Toggle invert */
	m_inverted = !m_inverted;
	/* Do memory toggle */
	for (uint16_t i = 0; i < sizeof(OLED_Buffer); i++) {
		OLED_Buffer[i] = ~OLED_Buffer[i];
	}
}
#endif

void OLED_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, OLED_Color_t color) {
	/* Check for overflow */
	if (x0 >= OLEDWIDTH) {
		x0 = OLEDWIDTH - 1;
	}
	if (x1 >= OLEDWIDTH) {
		x1 = OLEDWIDTH - 1;
	}
	if (y0 >= OLEDHEIGHT) {
		y0 = OLEDHEIGHT - 1;
	}
	if (y1 >= OLEDHEIGHT) {
		y1 = OLEDHEIGHT - 1;
	}

	// Bresenham's algorithm - thx wikpedia
	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			OLED_DrawPixel(y0, x0, color);
		} else {
			OLED_DrawPixel(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}

void OLED_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, OLED_Color_t color) {
	/* Check input parameters */
	if (x >= OLEDWIDTH || y >= OLEDHEIGHT) {
		/* Return error */
		return;
	}
	/* Check width and height */
	if ((x + w) >= OLEDWIDTH) {
		w = OLEDWIDTH - x;
	}
	if ((y + h) >= OLEDHEIGHT) {
		h = OLEDHEIGHT - y;
	}
	/* Draw 4 lines */
	OLED_DrawLine(x, y, x + w, y, color); /* Top line */
	OLED_DrawLine(x, y + h, x + w, y + h, color); /* Bottom line */
	OLED_DrawLine(x, y, x, y + h, color); /* Left line */
	OLED_DrawLine(x + w, y, x + w, y + h, color); /* Right line */
}

void OLED_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, OLED_Color_t color) {
	/* Check input parameters */
	if (x >= OLEDWIDTH || y >= OLEDHEIGHT) {
		/* Return error */
		return;
	}
	/* Check width and height */
	if ((x + w) >= OLEDWIDTH) {
		w = OLEDWIDTH - x;
	}
	if ((y + h) >= OLEDHEIGHT) {
		h = OLEDHEIGHT - y;
	}
	/* Draw lines */
	for (uint8_t i = 0; i <= h; i++) {
		/* Draw lines */
		OLED_DrawLine(x, y + i, x + w, y + i, color);
	}
}

void OLED_DrawCircle(int16_t x0, int16_t y0, int16_t r, OLED_Color_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	OLED_DrawPixel(x0, y0 + r, color);
	OLED_DrawPixel(x0, y0 - r, color);
	OLED_DrawPixel(x0 + r, y0, color);
	OLED_DrawPixel(x0 - r, y0, color);

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		OLED_DrawPixel(x0 + x, y0 + y, color);
		OLED_DrawPixel(x0 - x, y0 + y, color);
		OLED_DrawPixel(x0 + x, y0 - y, color);
		OLED_DrawPixel(x0 - x, y0 - y, color);
		OLED_DrawPixel(x0 + y, y0 + x, color);
		OLED_DrawPixel(x0 - y, y0 + x, color);
		OLED_DrawPixel(x0 + y, y0 - x, color);
		OLED_DrawPixel(x0 - y, y0 - x, color);
	}
}

void OLED_DrawCircleHelper(int16_t x0, int16_t y0, int16_t r, uint8_t cornername, OLED_Color_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			OLED_DrawPixel(x0 + x, y0 + y, color);
			OLED_DrawPixel(x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			OLED_DrawPixel(x0 + x, y0 - y, color);
			OLED_DrawPixel(x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			OLED_DrawPixel(x0 - y, y0 + x, color);
			OLED_DrawPixel(x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			OLED_DrawPixel(x0 - y, y0 - x, color);
			OLED_DrawPixel(x0 - x, y0 - y, color);
		}
	}
}

void OLED_FillCircle(int16_t x0, int16_t y0, int16_t r, OLED_Color_t color) {
	OLED_DrawLine(x0, y0 - r, x0, y0 + r + 1, color);
	OLED_FillCircleHelper(x0, y0, r, 3, 0, color);
}

void OLED_FillCircleHelper(uint16_t x0, uint16_t y0, uint16_t r, uint8_t cornername, int16_t delta, OLED_Color_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if (cornername & 0x1) {
			OLED_DrawLine(x0 + x, y0 - y, x0 + x, y0 + y + 1 + delta, color);
			OLED_DrawLine(x0 + y, y0 - x, x0 + y, y0 + x + 1 + delta, color);
		}
		if (cornername & 0x2) {
			OLED_DrawLine(x0 - x, y0 - y, x0 - x, y0 + y + 1 + delta, color);
			OLED_DrawLine(x0 - y, y0 - x, x0 - y, y0 + x + 1 + delta, color);
		}
	}
}

void OLED_DrawTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, OLED_Color_t color) {
	/* Draw lines */
	OLED_DrawLine(x0, y0, x1, y1, color);
	OLED_DrawLine(x1, y1, x2, y2, color);
	OLED_DrawLine(x2, y2, x0, y0, color);
}

void OLED_FillTriangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, OLED_Color_t color) {
	int16_t a, b, y, last;

	// Sort coordinates by Y order (y2 >= y1 >= y0)
	if (y0 > y1) {
		swap(y0, y1);
		swap(x0, x1);
	}
	if (y1 > y2) {
		swap(y2, y1);
		swap(x2, x1);
	}
	if (y0 > y1) {
		swap(y0, y1);
		swap(x0, x1);
	}

	if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
		a = b = x0;
		if (x1 < a)
			a = x1;
		else if (x1 > b)
			b = x1;
		if (x2 < a)
			a = x2;
		else if (x2 > b)
			b = x2;
		OLED_DrawLine(a, y0, b + 1, y0, color);
		return;
	}

	int16_t dx01 = x1 - x0, dy01 = y1 - y0, dx02 = x2 - x0, dy02 = y2 - y0, dx12 = x2 - x1, dy12 = y2 - y1;
	int32_t sa = 0, sb = 0;

	// For upper part of triangle, find scanline crossings for segments
	// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
	// is included here (and second loop will be skipped, avoiding a /0
	// error there), otherwise scanline y1 is skipped here and handled
	// in the second loop...which also avoids a /0 error here if y0=y1
	// (flat-topped triangle).
	if (y1 == y2)
		last = y1;   // Include y1 scanline
	else
		last = y1 - 1; // Skip it

	for (y = y0; y <= last; y++) {
		a = x0 + sa / dy01;
		b = x0 + sb / dy02;
		sa += dx01;
		sb += dx02;
		/* longhand:
		 a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
		 b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		 */
		if (a > b)
			swap(a, b);
		OLED_DrawLine(a, y, b + 1, y, color);
	}

	// For lower part of triangle, find scanline crossings for segments
	// 0-2 and 1-2.  This loop is skipped if y1=y2.
	sa = dx12 * (y - y1);
	sb = dx02 * (y - y0);
	for (; y <= y2; y++) {
		a = x1 + sa / dy12;
		b = x0 + sb / dy02;
		sa += dx12;
		sb += dx02;
		/* longhand:
		 a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
		 b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
		 */
		if (a > b)
			swap(a, b);
		OLED_DrawLine(a, y, b + 1, y, color);
	}
}
void OLED_DrawRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, OLED_Color_t color) {
	// smarter version
	OLED_DrawLine(x + r, y, x + w - r, y, color); // Top
	OLED_DrawLine(x + r, y + h - 1, x + w - r, y + h - 1, color); // Bottom
	OLED_DrawLine(x, y + r, x, y + h - r, color); // Left
	OLED_DrawLine(x + w - 1, y + r, x + w - 1, y + h - r, color); // Right
	// draw four corners
	OLED_DrawCircleHelper(x + r, y + r, r, 1, color);
	OLED_DrawCircleHelper(x + w - r - 1, y + r, r, 2, color);
	OLED_DrawCircleHelper(x + w - r - 1, y + h - r - 1, r, 4, color);
	OLED_DrawCircleHelper(x + r, y + h - r - 1, r, 8, color);
}
void OLED_FillRoundRect(int16_t x, int16_t y, int16_t w, int16_t h, int16_t r, OLED_Color_t color) {
	// smarter version
	OLED_FillRect(x + r, y, w - 2 * r, h, color);
	// draw four corners
	OLED_FillCircleHelper(x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	OLED_FillCircleHelper(x + r, y + r, r, 2, h - 2 * r - 1, color);
}

void OLED_DrawChar(int16_t x, int16_t y, uint8_t c, OLED_Color_t color, uint8_t fontindex) {
	uint16_t height, width, bytes;
	uint8_t offset;
	uint32_t charindex = 0;
	uint8_t *pchar;
	uint32_t line = 0;

	height = fonts[fontindex]->Height;
	width = fonts[fontindex]->Width;

	if ((x >= m_width) || // Clip right
		(y >= m_height) || // Clip bottom
		((x + width - 1) < 0) || // Clip left
		((y + height - 1) < 0))   // Clip top
		return;

	bytes = (width + 7) / 8;
	if (c < ' ') c = ' ';
#ifndef USE_CP1251
	else if (c > '~') c = ' ';
#endif
	charindex = (c - ' ') * height * bytes;
	offset = 8 * bytes - width;

	for (uint32_t i = 0; i < height; i++) {
		pchar = ((uint8_t *) &fonts[fontindex]->table[charindex] + (width + 7) / 8 * i);
		switch (bytes) {
		case 1:
			line = pchar[0];
			break;
		case 2:
			line = (pchar[0] << 8) | pchar[1];
			break;
		case 3:
		default:
			line = (pchar[0] << 16) | (pchar[1] << 8) | pchar[2];
			break;
		}
		for (uint32_t j = 0; j < width; j++) {
			if (line & (1 << (width - j + offset - 1))) {
				OLED_DrawPixel((x + j), y, color);
			} else {
				OLED_DrawPixel((x + j), y, !color);
			}
		}
		y++;
	}
}
void OLED_Printf(const char *fmt, ...) {
	static char buf[256];
	char *p;
	va_list lst;

	va_start(lst, fmt);
	vsprintf(buf, fmt, lst);
	va_end(lst);

	volatile uint16_t height, width;
	height = fonts[m_font]->Height;
	width = fonts[m_font]->Width;

	p = buf;
	while (*p) {
		if (*p == '\n') {
			m_cursor_y += height;
			m_cursor_x = 0;
		} else if (*p == '\r') {
			m_cursor_x = 0;
		} else if (*p == '\t') {
			m_cursor_x += width * 4;
		} else {
			if (m_cursor_x == 0) {
				OLED_FillRect(0, m_cursor_y, m_width - 1, m_cursor_y + height - 1, !m_textcolor);
			}
			if (m_cursor_y >= (m_height - height)) {
				m_cursor_y = 0;
				OLED_FillScreen(!m_textcolor);
			}
			OLED_DrawChar(m_cursor_x, m_cursor_y, *p, m_textcolor, m_font);
			m_cursor_x += width;
			if (m_wrap && (m_cursor_x > (m_width - width))) {
				m_cursor_y += height;
				m_cursor_x = 0;
			}
		}
		p++;
	}
}

void OLED_SetCursor(uint16_t x, uint16_t y) {
	/* Set write pointers */
	m_cursor_x = x;
	m_cursor_y = y;
}

void OLED_SetTextSize(uint8_t s) {
	if (s < 0) {
		m_font = 0;
	} else if (s >= fontsNum) {
		m_font = fontsNum - 1;
	} else {
		m_font = s;
	}
}

void OLED_SetTextColor(OLED_Color_t c) {
	m_textcolor = c;
}

void OLED_SetTextWrap(uint8_t w) {
	m_wrap = w;
}

int16_t OLED_GetCursorX(void) {
	return m_cursor_x;
}

int16_t OLED_GetCursorY(void) {
	return m_cursor_y;
}

void OLED_DisplayOn(void){
#if defined(SSD1306)
	OLED_WriteCmd(SSD1306_SETCHARGEPUMP);
	OLED_WriteCmd(SSD1306_CHARGEPUMP_ON);
	OLED_WriteCmd(SSD1306_SETDISPLAY_ON);
#elif defined(SH1106)
	OLED_WriteCmd(SH1106_SETDCDC);
	OLED_WriteCmd(SH1106_DCDC_ON);
	OLED_WriteCmd(SH1106_SETDISPLAY_ON);
#endif
}

void OLED_DisplayOff(void){
#if defined(SSD1306)
	OLED_WriteCmd(SSD1306_SETDISPLAY_OFF);
	OLED_WriteCmd(SSD1306_SETCHARGEPUMP);
	OLED_WriteCmd(SSD1306_CHARGEPUMP_OFF);
#elif defined(SH1106)
	OLED_WriteCmd(SH1106_SETDISPLAY_OFF);
	OLED_WriteCmd(SH1106_SETDCDC);
	OLED_WriteCmd(SH1106_DCDC_OFF);
#endif
}

#if defined(SSD1306)
void OLED_ScrollStartHorzRight(uint8_t startrow, uint8_t stoprow, uint8_t step) {
	OLED_WriteCmd(SSD1306_SCROLL_SET_VERT_AREA);
	OLED_WriteCmd(startrow);
	OLED_WriteCmd(stoprow);
	OLED_WriteCmd(SSD1306_SCROLL_HORZ_RIGHT);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(startrow / 8);
	OLED_WriteCmd(step);
	OLED_WriteCmd(stoprow / 8);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(0xFF);
	OLED_WriteCmd(SSD1306_SCROLL_ACTIVATE);
}

void OLED_ScrollStartHorzLeft(uint8_t startrow, uint8_t stoprow, uint8_t step) {
	OLED_WriteCmd(SSD1306_SCROLL_SET_VERT_AREA);
	OLED_WriteCmd(startrow);
	OLED_WriteCmd(stoprow);
	OLED_WriteCmd(SSD1306_SCROLL_HORZ_LEFT);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(startrow / 8);
	OLED_WriteCmd(step);
	OLED_WriteCmd(stoprow / 8);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(0xFF);
	OLED_WriteCmd(SSD1306_SCROLL_ACTIVATE);
}

void OLED_ScrollStartDiagRight(uint8_t startrow, uint8_t stoprow, uint8_t step, uint8_t vertoffset) {
	OLED_WriteCmd(SSD1306_SCROLL_SET_VERT_AREA);
	OLED_WriteCmd(startrow);
	OLED_WriteCmd(stoprow);
	OLED_WriteCmd(SSD1306_SCROLL_DIAG_RIGHT);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(startrow / 8);
	OLED_WriteCmd(step);
	OLED_WriteCmd(stoprow / 8);
	OLED_WriteCmd(vertoffset);
	OLED_WriteCmd(SSD1306_SCROLL_ACTIVATE);
}

void OLED_ScrollStartDiagLeft(uint8_t startrow, uint8_t stoprow, uint8_t step, uint8_t vertoffset) {
	OLED_WriteCmd(SSD1306_SCROLL_SET_VERT_AREA);
	OLED_WriteCmd(startrow);
	OLED_WriteCmd(stoprow);
	OLED_WriteCmd(SSD1306_SCROLL_DIAG_LEFT);
	OLED_WriteCmd(0x00);
	OLED_WriteCmd(startrow / 8);
	OLED_WriteCmd(step);
	OLED_WriteCmd(stoprow / 8);
	OLED_WriteCmd(vertoffset);
	OLED_WriteCmd(SSD1306_SCROLL_ACTIVATE);
}

void OLED_ScrollStop(void) {
	OLED_WriteCmd(SSD1306_SCROLL_DEACTIVATE);
}

void OLED_FadeStart(uint8_t mode, uint8_t step) {
	OLED_WriteCmd(SSD1306_SETFADEOUTANDBLINK);
	OLED_WriteCmd(mode | step);
}

void OLED_FadeStop(void) {
	OLED_WriteCmd(SSD1306_SETFADEOUTANDBLINK);
	OLED_WriteCmd(SSD1306_FADE_OFF);
}

#if OLEDHEIGHT > 32
void OLED_SetZoomIn(uint8_t enable) {
	OLED_WriteCmd(SSD1306_SETZOOMIN);
	OLED_WriteCmd(enable);
}
#endif
#endif
