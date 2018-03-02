#ifndef STRUCT_CONFIG_IODA_H
#define STRUCT_CONFIG_IODA_H
 
#include <stdint.h>

struct struct_config_ioda {  
  uint32_t  flashflag;
  uint32_t  autobackup;
  uint32_t  wdenable;
  uint32_t  wdtime;
  uint32_t  blreport;
  uint32_t  netsource;
  uint32_t  configured;
  uint32_t  launched;
  char		alias[64];
  uint32_t	_dummy1; // previously "trusted"
  uint32_t	_dummy2; // previously "backuptime"
  uint32_t	webview;
  uint32_t	webport;
  int		timeoffset;
  uint32_t	timesync;
  uint32_t	lowpanbr;
  uint32_t	restartbl;
  uint32_t	autojump;
};

#endif /* STRUCT_CONFIG_IODA_H */ 
