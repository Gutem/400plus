#include "main.h"
#include "firmware.h"

#include "menu.h"
#include "languages.h"

#include "menuitem.h"

OPTIONLIST_DEF(bool,     LP_WORD(L_NO), LP_WORD(L_YES))
OPTIONLIST_DEF(delay,    LP_WORD(L_NO), LP_WORD(L_2S))
OPTIONLIST_DEF(flash,    LP_WORD(L_ENABLED), LP_WORD(L_DISABLED), LP_WORD(L_EXT_ONLY))
OPTIONLIST_DEF(action,   LP_WORD(L_ONE_SHOT), LP_WORD(L_EXT_AEB), LP_WORD(L_INTERVAL))
OPTIONLIST_DEF(logfile,  LP_WORD(L_OVERWRITE), LP_WORD(L_NEW), LP_WORD(L_APPEND));

void menuitem_print(const char *buffer, const char *name, const char *parameter);

void menuitem_display(const type_MENUITEM *item, const char *buffer) {
	menuitem_print(buffer, item->name, "");
}

void menuitem_display_ev(const type_MENUITEM *item, const char *buffer) {
	const char value[LP_MAX_WORD];

	if (item->parm.menuitem_ev.zero_means_off && *item->parm.menuitem_ev.value == 0) {
		menuitem_print(buffer, item->name, LP_WORD(L_OFF));
	} else {
		ev_print(value, *item->parm.menuitem_ev.value);
		menuitem_print(buffer, item->name, value);
	}
}

void menuitem_display_av(const type_MENUITEM *item, const char *buffer) {
	const char value[LP_MAX_WORD];

	av_print(value, *item->parm.menuitem_av.value);
	menuitem_print(buffer, item->name, value);
}

void menuitem_display_tv(const type_MENUITEM *item, const char *buffer) {
	const char value[LP_MAX_WORD];

	tv_print(value, *item->parm.menuitem_tv.value);
	menuitem_print(buffer, item->name, value);
}

void menuitem_display_iso(const type_MENUITEM *item, const char *buffer) {
	const char value[LP_MAX_WORD];

	iso_print(value, *item->parm.menuitem_iso.value);
	menuitem_print(buffer, item->name, value);
}

void menuitem_display_int(const type_MENUITEM *item, const char *buffer) {
	const char value[LP_MAX_WORD];

	if (item->parm.menuitem_int.zero_means_unlimited && *item->parm.menuitem_int.value == 0) {
		menuitem_print(buffer, item->name, LP_WORD(L_NO_LIMIT));
	} else {
		sprintf(value, item->parm.menuitem_int.format, *item->parm.menuitem_int.value);
		menuitem_print(buffer, item->name, value);
	}
}

void menuitem_display_enum(const type_MENUITEM *item, const char *buffer) {
	menuitem_print(buffer, item->name, item->parm.menuitem_enum.list->data[*item->parm.menuitem_enum.value]);
}

void menuitem_display_sub(const type_MENUITEM *item, const char *buffer) {
	menuitem_print(buffer, item->name, ">");
}

void menuitem_print(const char *buffer, const char *name, const char *parameter) {
	const int pad = MENU_WIDTH - strlen(parameter) - 2;

	sprintf(buffer, "%-*.*s %s", pad, pad, name, parameter);
}

void menuitem_right(const type_MENUITEM *item, const int repeating) {
	switch(item->type) {
	case MENUITEM_TYPE_EV:
		*item->parm.menuitem_ev.value = ev_inc(*item->parm.menuitem_ev.value);
		break;
	case MENUITEM_TYPE_AV:
		*item->parm.menuitem_av.value = av_inc(*item->parm.menuitem_av.value);
		break;
	case MENUITEM_TYPE_TV:
		if (repeating || item->parm.menuitem_tv.bulb)
			*item->parm.menuitem_tv.value = tv_next(*item->parm.menuitem_tv.value);
		else
			*item->parm.menuitem_tv.value = tv_inc(*item->parm.menuitem_tv.value);
		break;
	case MENUITEM_TYPE_ISO:
		if (repeating || item->parm.menuitem_iso.full)
			*item->parm.menuitem_iso.value = iso_next(*item->parm.menuitem_iso.value);
		else
			*item->parm.menuitem_iso.value = iso_inc(*item->parm.menuitem_iso.value);
		break;
	case MENUITEM_TYPE_INT:
		*item->parm.menuitem_int.value += repeating ? item->parm.menuitem_int.big_step : item->parm.menuitem_int.small_step;
		*item->parm.menuitem_int.value  = MIN(*item->parm.menuitem_int.value, item->parm.menuitem_int.max);
		break;
	case MENUITEM_TYPE_ENUM:
		if (*item->parm.menuitem_enum.value == item->parm.menuitem_enum.list->length - 1) {
			if (item->parm.menuitem_enum.cycle)
				*item->parm.menuitem_enum.value = 0;
		} else
			(*item->parm.menuitem_enum.value)++;
		break;
	case MENUITEM_TYPE_SUBMENU:
		if (!repeating)
			menu_set_page(item->parm.menuitem_submenu.page);
		break;
	default:
		break;
	}
}

void menuitem_left(const type_MENUITEM *item, const int repeating) {
	switch(item->type) {
	case MENUITEM_TYPE_EV:
		if (item->parm.menuitem_ev.zero_means_off && *item->parm.menuitem_ev.value < 0x05)
			*item->parm.menuitem_ev.value = 0x00;
		else
			*item->parm.menuitem_ev.value = ev_dec(*item->parm.menuitem_ev.value);
		break;
	case MENUITEM_TYPE_AV:
		*item->parm.menuitem_av.value = av_dec(*item->parm.menuitem_av.value);
		break;
	case MENUITEM_TYPE_TV:
		if (repeating || item->parm.menuitem_tv.bulb)
			*item->parm.menuitem_tv.value = tv_prev(*item->parm.menuitem_tv.value);
		else
			*item->parm.menuitem_tv.value = tv_dec(*item->parm.menuitem_tv.value);
		break;
	case MENUITEM_TYPE_ISO:
		if (repeating || item->parm.menuitem_iso.full)
			*item->parm.menuitem_iso.value = iso_prev(*item->parm.menuitem_iso.value);
		else
			*item->parm.menuitem_iso.value = iso_dec(*item->parm.menuitem_iso.value);
		break;
	case MENUITEM_TYPE_INT:
		*item->parm.menuitem_int.value -= repeating ? item->parm.menuitem_int.big_step : item->parm.menuitem_int.small_step;
		*item->parm.menuitem_int.value  = MAX(*item->parm.menuitem_int.value, item->parm.menuitem_int.min);
		break;
	case MENUITEM_TYPE_ENUM:
		if (*item->parm.menuitem_enum.value == 0) {
			if (item->parm.menuitem_enum.cycle)
				*item->parm.menuitem_enum.value = item->parm.menuitem_enum.list->length - 1;
		} else
			*item->parm.menuitem_enum.value -= 1;
		break;
	default:
		break;
	}
}

void menuitem_cycle(const type_MENUITEM *item, const int repeating) {
	switch(item->type) {
	case MENUITEM_TYPE_EV:
		if (!item->parm.menuitem_ev.zero_means_off)
			*item->parm.menuitem_ev.value = ev_sgn(*item->parm.menuitem_ev.value);
		break;
	case MENUITEM_TYPE_ISO:
		if (repeating)
			*item->parm.menuitem_iso.value = iso_next(*item->parm.menuitem_iso.value);
		else
			*item->parm.menuitem_iso.value = iso_inc(*item->parm.menuitem_iso.value);
		break;
	case MENUITEM_TYPE_INT:
		*item->parm.menuitem_int.value += repeating ? item->parm.menuitem_int.big_step : item->parm.menuitem_int.small_step;
		*item->parm.menuitem_int.value  = MIN(*item->parm.menuitem_int.value, item->parm.menuitem_int.max);
		break;
	case MENUITEM_TYPE_ENUM:
		if (*item->parm.menuitem_enum.value == item->parm.menuitem_enum.list->length - 1)
			*item->parm.menuitem_enum.value = 0;
		else
			*item->parm.menuitem_enum.value += 1;
		break;
	default:
		break;
	}
}