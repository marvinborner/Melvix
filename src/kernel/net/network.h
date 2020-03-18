#ifndef MELVIX_NETWORK_H
#define MELVIX_NETWORK_H

void rtl8139_install();

void network_install()
{
#ifdef rtl8139
	rtl8139_install();
#endif
}

#endif
