#include "HTMLParser.h"

REGISTER_DEBUG(HTMLParser);

char*				HTMLParser::_buffer;
ExtMem*				HTMLParser::_extmem;

string header;
string menu;
string footer;

void HTMLParser::init(ExtMem* extmem) {

	_extmem = extmem;
	_buffer = new char[64];

	int rc = 0;

	// create menu
	menu += "<table cellspacing='10px'><tr>";
	menu += "<th><h2>";
	rc = Configurator::get_alias(_buffer);
	if(!rc){
		menu += _buffer;
	}
	menu += "</h2></th>";
	menu += "<th><h2><a href=\"/\">General</a></h2></th>";
	menu += "<th><h2><a href=\"/binmanager\">Bin manager</a></h2></th>";
	menu += "<th><h2><a href=\"/threads\">Threads</a></h2></th>";
	menu += "<th><h2><a href=\"/inputs\">Inputs</a></h2></th>";
	menu += "<th><h2><a href=\"/mqtt\">MQTT</a></h2></th>";
	menu += "<th><h2><a href=\"/configurator\">Configurator</a></h2></th>";
	menu += "</tr></table>";

	// create header
	header += "<html><head><title>";
	sprintf(_buffer, "%s | ", TOSTRING_TARGET(__BUILD_TARGET__));
	header += _buffer;
	rc = Configurator::get_alias(_buffer);
	if(!rc){
		header += _buffer;
	}
	header += "</title></head><body>";

	// create footer
	footer += "<br><hr>Copyright Byzance | ";
	sprintf(_buffer, "Compiled on %02d. %02d. %04d - %02d:%02d:%02d | ", __BUILD_DAY__, __BUILD_MONTH__, __BUILD_YEAR_LEN4__, __BUILD_HOUR__, __BUILD_MINUTE__, __BUILD_SECOND__);
	footer += _buffer;
	sprintf(_buffer, "ARM GCC NONEABI = %s.%s.%s | ", TOSTRING(__GNUC__), TOSTRING(__GNUC_MINOR__), TOSTRING(__GNUC_PATCHLEVEL__));
	footer += _buffer;
	sprintf(_buffer, "MBED = %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
	footer += _buffer;
	footer += "<hr></body></html>";

}

HttpError_t HTMLParser::parse(char* request, string& response) {

	uint32_t tmp32;
	float tmpf;

	__DEBUG("received REQUEST");

	/************************************************
	 * ROOT
	 ************************************************/
	if (strcmp(request, HTML_PAGE_HOME) == 0) {

		__INFO("requesting general\n");

		response += header;
		response += "<script>setTimeout(function(){window.location.reload(1);}, 1000);</script>";
		response += menu;

		response += "<h1>General</h1>";

		response += "<div style=\"color:#FF0000\">Note: this page is self-refreshing every second</div>";

		response += "<h2>Miscellaneous</h2>";
		response += "<table cellspacing='10px'>\n";
		response += "<tr><td  width='150px'>Hardware revision</td><td>";
		sprintf(_buffer, "0x%08X", (unsigned int)get_revision());
		response += _buffer;
		response += "</td></tr>";

		response += "<tr><td>Supply voltage</td><td>";
		Byzance::get_supply_voltage(&tmpf);
		sprintf(_buffer, "%3.3f (V)", tmpf);
		response += _buffer;
		response += "</td></tr>";

		response += "<tr><td>Vref internal</td><td>";
		Byzance::get_vref_int(&tmpf);
		sprintf(_buffer, "%3.3f (V)", tmpf);
		response += _buffer;
		response += "</td></tr>";

		response += "<tr><td>Core temperature</td><td>";
		Byzance::get_core_temp(&tmpf);
		sprintf(_buffer, "%3.3f (C)", tmpf);
		response += _buffer;
		response += "</td></tr>";

		response += "<tr><td>mac_address</td><td>";
		char mac_addr[64];
		get_mac_address(mac_addr);
		response+=mac_addr;
		response+="</td></tr>\n";
		response+="<tr><td>full_id</td><td>";
		response+=ByzanceCore::_fullID;
		response+="</td></tr>\n";
		response+="<tr><td>web console</td><td>";
		if(Console::enabled()){
			response += "<div style=\"color:green\">enabled</div>";
		} else {
			response += "<div style=\"color:red\">disabled</div>";
		}
		response+="</td></tr>\n";
		response += "</table>";


		response += "<h2>Homer</h2>";
		response += "<table cellspacing='10px'>";
		response+="<tr><td width='150px'>homer status</td><td>";
		if(ByzanceClient::is_connected()){
			response += "<div style=\"color:green\">connected</div>";
		} else {
			response += "<div style=\"color:red\">disconnected</div>";
		}
		response+="</td></tr>\n";

		response += "<tr><td>Connected timer</td><td>";
		sprintf(_buffer, "%u", (unsigned int)ByzanceCore::connected_time());
		response += _buffer;
		response += "</td></tr>";

		response += "</table>";

		response += "<h2>Time</h2>";
		response += "<table cellspacing='10px'>";

		response += "<tr><td width='150px'>Timestamp</td><td>";
		time_t timestamp = 0;
		time(&timestamp);
		sprintf(_buffer, "%u", (unsigned int)timestamp);
		response += _buffer;
		response += "</td></tr>";

		response += "<tr><td>Uptime</td><td>";
		sprintf(_buffer, "%u", (unsigned int)ByzanceCore::uptime);
		response += _buffer;
		response += "</td></tr>";
		response += "</table>";

		response += "<h2>Restart</h2>";
		response += "<table cellspacing='10px'>";
		response+="<tr><td width='150px'>Pending</td><td>";
		if(ByzanceCore::restart_pending){
			response += "<div style=\"color:green\">true</div>";
		} else {
			response += "<div style=\"color:red\">false</div>";
		}
		response+="</td></tr>";

		response += "<tr><td>counter:</td><td>";
		sprintf(_buffer, "%d\n", (int)ByzanceCore::restart_sec);
		response += _buffer;
		response += "</td></tr>";
		response += "</table>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * BINARY MANAGER
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_BINMANAGER) == 0) {

		__INFO("requesting binmanager\n");

		response += header;
		response += "<script>setTimeout(function(){window.location.reload(1);}, 1000);</script>";
		response += menu;

		response += "<h1>Binary manager</h1>";

		response += "<div style=\"color:#FF0000\">Note: this page is self-refreshing every second</div>";



		// outer table
		response += "<table cellspacing='5px'><tr><td>";

		// firmware
		response += "<h2>Firmware version</h2>\n";
		HTMLParser::show_binary(BIN_COMPONENT_FIRMWARE, response);
		response += "</td><td>\n";

		// backup
		response += "<h2>Backup version</h2>\n";
		HTMLParser::show_binary(BIN_COMPONENT_BACKUP, response);
		response += "</td></tr><tr><td>\n";

		// bootloader
		response += "<h2>Bootloader version</h2>";
		HTMLParser::show_binary(BIN_COMPONENT_BOOTLOADER, response);
		response += "</td><td>\n";

		// buffer
		response += "<h2>Buffer version</h2>";
		HTMLParser::show_binary(BIN_COMPONENT_BUFFER, response);

		response += "</td></tr></table>\n";

		response += footer;

		return HTTP_200_OK;

		/************************************************
		 * THREADS
		 ************************************************/
	} else if (strcmp(request, HTML_PAGE_THREADS) == 0) {

		__INFO("requesting threads\n");

		// vyrobim odpoved
		response += header;
		response += "<script>setTimeout(function(){window.location.reload(1);}, 1000);</script>";
		response += menu;
		response += "<h1>Threads</h1>";

		response += "<div style=\"color:#FF0000\">Note: this page is self-refreshing every second</div>";

		/*
		 * CPU load
		 */

		response +="<h2>CPU load</h2>";
		if(ByzanceCore::cpu_load<50){
			// zelená
			response+= "<div style=\"color:#006400; font-size:50px\">";
		} else if (ByzanceCore::cpu_load > 90) {
			// červená
			response+="<div style=\"color:#FF0000; font-size:50px\">";
		} else {
			// oranžová
			response+= "<div style=\"color:#FFA500; font-size:50px\">";
		}

		sprintf(_buffer, "%d %%", ByzanceCore::cpu_load);
		response += _buffer;
		response += "</div>";

#if HTML_SHOW_THREAD_BYZANCE
		/****************************
		 * BYZANCE THREAD
		 ****************************/

		response += "<h2>Byzance thread overview</h2>\n";
		response += "<ul>\n";

		/*
		 * byzance stack size
		 */
		sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) Byzance::get_stack_size());
		response += _buffer;

		/*
		 * byzance free stack
		 */
		// tady jsou rovnitka naopak, protoze cim vic free, tim lip
		tmp32 = (Byzance::get_free_stack() * 100) / Byzance::get_stack_size();
		if (tmp32 < 30) {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		} else if (tmp32 < 70) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		}
		response += _buffer;
		sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>", (int) Byzance::get_free_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * byzance used stack
		 */
		tmp32 = (Byzance::get_used_stack() * 100) / Byzance::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) Byzance::get_used_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * byzance max stack
		 */
		tmp32 = (Byzance::get_max_stack() * 100) / Byzance::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) Byzance::get_max_stack(), (int) tmp32);
		response += _buffer;

		response +="<li>state=";
		get_name_state(Byzance::get_state(), _buffer);
		response+=_buffer;
		response+="</li>";

		response+="<li>priority=";
		get_name_priority(Byzance::get_priority(), _buffer);
		response+=_buffer;
		response+="</li>";

		response += "</ul>\n";
#endif

#if HTML_SHOW_THREAD_CLIENT
		/****************************
		 * CLIENT THREAD
		 ****************************/
		response += "<h2>MQTT Client thread overview</h2>\n";
		response += "<ul>\n";

		/*
		 * client stack size
		 */
		sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) ByzanceClient::get_stack_size());
		response += _buffer;

		/*
		 * client free stack
		 */
		// tady jsou rovnitka naopak, protoze cim vic free, tim lip
		tmp32 = (ByzanceClient::get_free_stack() * 100) / ByzanceClient::get_stack_size();
		if (tmp32 < 30) {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		} else if (tmp32 < 70) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		}
		response += _buffer;
		sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>\n", (int) ByzanceClient::get_free_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * client used stack
		 */
		tmp32 = (ByzanceClient::get_used_stack() * 100) / ByzanceClient::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) ByzanceClient::get_used_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * client max stack
		 */
		tmp32 = (ByzanceClient::get_max_stack() * 100) / ByzanceClient::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) ByzanceClient::get_max_stack(), (int) tmp32);
		response += _buffer;

		response +="<li>state=";
		get_name_state(ByzanceClient::get_state(), _buffer);
		response+=_buffer;
		response+="</li>";

		response+="<li>priority=";
		get_name_priority(ByzanceClient::get_priority(), _buffer);
		response+=_buffer;
		response+="</li>";

		response += "</ul>\n";
#endif

#if HTML_SHOW_THREAD_LOWPANBR

		/****************************
		 * LOWPANBR THREAD
		 ****************************/

		response += "<h2>LowpanBR thread overview</h2>\n";

		uint32_t rc = 0;

		rc = Configurator::get_lowpanbr(&tmp32);
		if(rc){

			response+=">error";

		} else if(tmp32){

			response += "<ul>\n";

			/*
			 * sender stack size
			 */
			sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) LowpanBR::get_stack_size());
			response += _buffer;

			/*
			 * sender free stack
			 */
			// tady jsou rovnitka naopak, protoze cim vic free, tim lip
			tmp32 = (LowpanBR::get_free_stack() * 100) / LowpanBR::get_stack_size();
			if (tmp32 < 30) {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			} else if (tmp32 < 70) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			}
			response += _buffer;
			sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>\n", (int) LowpanBR::get_free_stack(), (int) tmp32);
			response += _buffer;

			/*
			 * lowpan used stack
			 */
			tmp32 = (LowpanBR::get_used_stack() * 100) / LowpanBR::get_stack_size();
			if (tmp32 < 70) {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			} else if (tmp32 < 90) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			}
			response += _buffer;
			sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) LowpanBR::get_used_stack(), (int) tmp32);
			response += _buffer;

			/*
			 * lowpan max stack
			 */
			tmp32 = (LowpanBR::get_max_stack() * 100) / LowpanBR::get_stack_size();
			if (tmp32 < 70) {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			} else if (tmp32 < 90) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			}
			response += _buffer;
			sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) LowpanBR::get_max_stack(), (int) tmp32);
			response += _buffer;

			//heap info
			const mem_stat_t* heap_info = LowpanBR::get_heap_info();
			/*
			 * heap size
			 */
			if(heap_info != NULL){
				sprintf(_buffer, "<li>heap_size = %d</li>\n", (int)heap_info->heap_sector_size);
				response += _buffer;
				//used heap
				tmp32 = (heap_info->heap_sector_allocated_bytes * 100) / heap_info->heap_sector_size;
				if (tmp32 < 70) {
					// zelená
					sprintf(_buffer, "<li><div style=\"color:#006400\">");
				} else if (tmp32 < 90) {
					// oranžová
					sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
				} else {
					// červená
					sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
				}
				response += _buffer;
				sprintf(_buffer, "used_heap = %d (%d  %%)</div></li>", (int) heap_info->heap_sector_allocated_bytes, (int) tmp32);
				response += _buffer;
				//used heap
				tmp32 = (heap_info->heap_sector_allocated_bytes_max * 100) / heap_info->heap_sector_size;
				if (tmp32 < 70) {
					// zelená
					sprintf(_buffer, "<li><div style=\"color:#006400\">");
				} else if (tmp32 < 90) {
					// oranžová
					sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
				} else {
					// červená
					sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
				}
				response += _buffer;
				sprintf(_buffer, "max_used_heap = %d (%d  %%)</div></li>", (int) heap_info->heap_sector_allocated_bytes_max, (int) tmp32);
				response += _buffer;
				sprintf(_buffer, "<li>allocation_failed = %d</li>\n", (int)heap_info->heap_alloc_fail_cnt);
				response += _buffer;
				const nat_nwk_statistic_t *nwk_stat = LowpanBR::get_nat_stat();
				sprintf(_buffer, "<li>eth_statistics OK/ERROR = %d/%d</li>\n", nwk_stat->eth_ok,nwk_stat->eth_ok);
				response += _buffer;
				sprintf(_buffer, "<li>lowpan_statistics OK/ERROR = %d/%d</li>\n", nwk_stat->lowpan_ok,nwk_stat->lowpan_fail);
				response += _buffer;
			}


			response +="<li>state=";
			get_name_state(LowpanBR::get_thread_state(), _buffer);
			response+=_buffer;
			response+="</li>";

			response+="<li>priority=";
			get_name_priority(LowpanBR::get_priority(), _buffer);
			response+=_buffer;
			response+="</li>";

			response += "</ul>\n";

		} else {
			response += "lowpanbr feature disabled";
		}
#endif

#if HTML_SHOW_THREAD_SENDER
		/****************************
		 * SENDER THREAD
		 ****************************/

		response += "<h2>SENDER thread overview</h2>\n";
		response += "<ul>\n";

		/*
		 * sender stack size
		 */
		sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) Sender::get_stack_size());
		response += _buffer;

		/*
		 * sender free stack
		 */
		// tady jsou rovnitka naopak, protoze cim vic free, tim lip
		tmp32 = (Sender::get_free_stack() * 100) / Sender::get_stack_size();
		if (tmp32 < 30) {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		} else if (tmp32 < 70) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		}
		response += _buffer;
		sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>\n", (int) Sender::get_free_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * sender used stack
		 */
		tmp32 = (Sender::get_used_stack() * 100) / Sender::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) Sender::get_used_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * sender max stack
		 */
		tmp32 = (Sender::get_max_stack() * 100) / Sender::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) Sender::get_max_stack(), (int) tmp32);
		response += _buffer;

		response +="<li>state=";
		get_name_state(Sender::get_thread_state(), _buffer);
		response+=_buffer;
		response+="</li>";

		response+="<li>priority=";
		get_name_priority(Sender::get_priority(), _buffer);
		response+=_buffer;
		response+="</li>";

		response += "</ul>\n";
#endif

#if HTML_SHOW_THREAD_HTTP
	/****************************
	 * HTTP SERVER THREAD
	 ****************************/

		response += "<h2>HTTP thread overview</h2>\n";
		response += "<ul>\n";

		/*
		 * server stack size
		 */
		sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) HttpServer::get_stack_size());
		response += _buffer;

		/*
		 * server free stack
		 */
		// tady jsou rovnitka naopak, protoze cim vic free, tim lip
		tmp32 = (HttpServer::get_free_stack() * 100) / HttpServer::get_stack_size();
		if (tmp32 < 30) {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		} else if (tmp32 < 70) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		}
		response += _buffer;
		sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>\n", (int) HttpServer::get_free_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * server used stack
		 */
		tmp32 = (HttpServer::get_used_stack() * 100) / HttpServer::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) HttpServer::get_used_stack(), (int) tmp32);
		response += _buffer;

		/*
		 * server max stack
		 */
		tmp32 = (HttpServer::get_max_stack() * 100) / HttpServer::get_stack_size();
		if (tmp32 < 70) {
			// zelená
			sprintf(_buffer, "<li><div style=\"color:#006400\">");
		} else if (tmp32 < 90) {
			// oranžová
			sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
		} else {
			// červená
			sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
		}
		response += _buffer;
		sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) HttpServer::get_max_stack(), (int) tmp32);
		response += _buffer;

		response +="<li>state=";
		get_name_state(HttpServer::get_state(), _buffer);
		response+=_buffer;
		response+="</li>";

		response+="<li>priority=";
		get_name_priority(HttpServer::get_priority(), _buffer);
		response+=_buffer;
		response+="</li>";

		response += "</ul>\n";
#endif

#if HTML_SHOW_THREAD_WIFI
	/****************************
	 * WIFI THREAD
	 ****************************/

		response += "<h2>WIFI thread overview</h2>\n";

		if (_netsource == NETSOURCE_WIFI) {

			/*
			 * WIFI communication part
			 */
			response += "<h3>Communication part</h3>\n";
			response += "<ul>\n";

			/*
			 * WIFI communication stack size
			 */
			sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) _wifi->get_stack_size_communication());
			response += _buffer;

			/*
			 * WIFI communication free stack
			 */
			// tady jsou rovnitka naopak, protoze cim vic free, tim lip
			tmp32 = (_wifi->get_free_stack_communication() * 100) / _wifi->get_stack_size_communication();
			if (tmp32 < 30) {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			} else if (tmp32 < 70) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			}
			response += _buffer;
			sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>\n", (int) _wifi->get_free_stack_communication(), (int) tmp32);
			response += _buffer;

			/*
			 * WIFI communication used stack
			 */
			tmp32 = (_wifi->get_used_stack_communication() * 100) / _wifi->get_stack_size_communication();
			if (tmp32 < 70) {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			} else if (tmp32 < 90) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			}
			response += _buffer;
			sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) _wifi->get_used_stack_communication(), (int) tmp32);
			response += _buffer;

			/*
			 * WIFI communication max stack
			 */
			tmp32 = (_wifi->get_max_stack_communication() * 100) / _wifi->get_stack_size_communication();
			if (tmp32 < 70) {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			} else if (tmp32 < 90) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			}
			response += _buffer;
			sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) _wifi->get_max_stack_communication(), (int) tmp32);
			response += _buffer;

			response +="<li>state=";
			get_name_state(_wifi->get_state_communication(), _buffer);
			response+=_buffer;
			response+="</li>";

			response+="<li>priority=";
			get_name_priority(_wifi->get_priority_communication(), _buffer);
			response+=_buffer;
			response+="</li>";

			response += "</ul>\n";

			/*
			 * WIFI logic part
			 */
			response += "<h3>Logic part</h3>\n";
			response += "<ul>\n";

			/*
			 * WIFI logic stack size
			 */
			sprintf(_buffer, "<li>stack_size = %d</li>\n", (int) _wifi->get_stack_size_logic());
			response += _buffer;

			/*
			 * WIFI logic free stack
			 */
			// tady jsou rovnitka naopak, protoze cim vic free, tim lip
			tmp32 = (_wifi->get_free_stack_logic() * 100) / _wifi->get_stack_size_logic();
			if (tmp32 < 30) {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			} else if (tmp32 < 70) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			}
			response += _buffer;
			sprintf(_buffer, "free_stack = %d (%d  %%)</div></li>\n", (int) _wifi->get_free_stack_logic(), (int) tmp32);
			response += _buffer;

			/*
			 * WIFI logic used stack
			 */
			tmp32 = (_wifi->get_used_stack_logic() * 100) / _wifi->get_stack_size_logic();
			if (tmp32 < 70) {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			} else if (tmp32 < 90) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			}
			response += _buffer;
			sprintf(_buffer, "used_stack = %d (%d  %%)</div></li>", (int) _wifi->get_used_stack_logic(), (int) tmp32);
			response += _buffer;

			/*
			 * WIFI logic max stack
			 */
			tmp32 = (_wifi->get_max_stack_logic() * 100) / _wifi->get_stack_size_logic();
			if (tmp32 < 70) {
				// zelená
				sprintf(_buffer, "<li><div style=\"color:#006400\">");
			} else if (tmp32 < 90) {
				// oranžová
				sprintf(_buffer, "<li><div style=\"color:#FFA500\">");
			} else {
				// červená
				sprintf(_buffer, "<li><div style=\"color:#FF0000\">");
			}
			response += _buffer;
			sprintf(_buffer, "max_stack = %d (%d  %%)</div></li>", (int) _wifi->get_max_stack_logic(), (int) tmp32);
			response += _buffer;

			response +="<li>state=";
			get_name_state(_wifi->get_state_logic(), _buffer);
			response+=_buffer;
			response+="</li>";

			response+="<li>priority=";
			get_name_priority(_wifi->get_priority_logic(), _buffer);
			response+=_buffer;
			response+="</li>";

			response += "</ul>\n";

		} else if (_netsource == NETSOURCE_ETHERNET) {
			response += "available only if wifi is enabled";
		}

#endif /* HTML_SHOW_THREAD_WIFI */

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * FAVICON
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_FAVICON) == 0) {

		__INFO("requesting favicon\n");

		return HTTP_404_NOT_FOUND;

	/************************************************
	 * WIFI
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_WIFI) == 0) {

		__INFO("requesting wifi\n");

		// vyrobim odpoved
		response += header;
		response += menu;
		response += "<h1>Wifi</h1>";

		response += footer;

		return HTTP_200_OK;

#if BYZANCE_SUPPORTS_PROTOCOL
	/************************************************
	 * DEVICES
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_DEVICES) == 0) {

		__INFO("requesting devices\n");

		// vyrobim odpoved
		response += header;
		response += "<script>setTimeout(function(){window.location.reload(1);}, 1200);</script>";
		response += menu;
		response += "<h1>Devices</h1>";

		PARSER_TRACE("ceka se na lock\n");
		if (DevList::lock() == DEVLIST_OK) {

			PARSER_TRACE("locked\n");

			uint32_t size = 0;
			int16_t short_id = 0;
			char* full_id = NULL;
			ItfEnum_TypeDef itf = ITF_UNKNOWN;
			bool saved = false;
			StateEnum_TypeDef state = DEV_STATE_UNKNOWN;
			unsigned char timeout_cnt = 0;
			long int time = 0;
			char alias[32];			// todo: dodat #define velikosti



			size = DevList::get_list_length();
			sprintf(_buffer, "<h2>Device list in Ioda (%d/%d)</h2>", (int) size, (int) DevList::get_list_capacity());
			response += _buffer;

			sprintf(_buffer, "<div style=\"font-family: monospace; white-space: pre;\">[##] Short / Full ID                   Interface             State              (State code)</div>");
			response += _buffer;
			for (int i = 0; i < size; i++) {
				// Format radku
				sprintf(_buffer, "<div style=\"font-family: monospace; white-space: pre;\">");
				response += _buffer;

				// Index
				sprintf(_buffer, "[%2d] ", i);
				response += _buffer;

				// Short ID
				if (DevList::get_short_id_at_index(&short_id, i) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				sprintf(_buffer, "0x%03X   ", short_id);
				response += _buffer;

				// Full ID
				if (DevList::get_full_id_at_index(&full_id, i) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				sprintf(_buffer, "%s  ", full_id);
				response += _buffer;


				// Interface
				if (DevList::get_itf_at_index(&itf, i) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				switch (itf) {
					case ITF_UNKNOWN:
						response += "<span style=\"color:#FF0000;\">unknown</span>  ";
						break;
					case ITF_NRF:
						response += "<span style=\"color:#00D000;\">NRF905 </span>  ";
						break;
					case ITF_RS485:
						response += "<span style=\"color:#0000FF;\">RS485  </span>  ";
						break;
					default:
						response += "<span style=\"color:#FF0000;\">error  </span>  ";
				}


				// Saved
				if(DevList::is_saved_at_index(&saved, i) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				response += "saved=";
				if (saved) {
					response += "<span style=\"color:#00D000;\">true  </span> ";
				} else {
					response += "<span style=\"color:#FF0000;\">false </span> ";
				}


				// State
				if(DevList::get_state_at_index(&state, i) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				response += "state=";
				switch (state) {
					case DEV_STATE_UNKNOWN:
						response += "<span style=\"color:#FF0000;\">unknown      </span>";
						break;
					case DEV_STATE_DISCONNECTED:
						response += "<span style=\"color:#FF0000;\">disconnected </span>";
						break;
					case DEV_STATE_ADD:
						response += "<span style=\"color:#0000FF;\">add          </span>";
						break;
					case DEV_STATE_ALIVE:
						response += "<span style=\"color:#0000FF;\">alive        </span>";
						break;
					case DEV_STATE_ENUMERATED:
						response += "<span style=\"color:#00D000;\">enumerated   </span>";
						break;
					case DEV_STATE_REMOVE:
						response += "<span style=\"color:#0000FF;\">remove       </span>";
						break;
					case DEV_STATE_TIMEOUT:
						response += "<span style=\"color:#FF0000;\">timeout      </span>";
						break;
					case DEV_STATE_DEAD:
						response += "<span style=\"color:#FF0000;\">dead          </span>";
						break;
					case DEV_STATE_LOST:
						response += "<span style=\"color:#FF0000;\">lost         </span>";
						break;
					default:
						response += "<span style=\"color:#FF0000;\">error        </span>";
				}

				// State jeste ciselne.
				sprintf(_buffer, "(0x%02X) ", state);
				response += _buffer;

				// Activity_cntr
				unsigned char activity_cntr = 0;
				DevList::get_activity_cntr_at_index(&activity_cntr, i);
				sprintf(_buffer, " activity_cntr=%2d", activity_cntr);
				response += _buffer;


				// Timeout_cnt
				if(DevList::get_timeout_cnt(&timeout_cnt, full_id) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				sprintf(_buffer, " timeout_cnt=%2d", timeout_cnt);
				response += _buffer;

				// Alias
				if (DevList::get_alias(alias, full_id) == DEVLIST_OK) {
					sprintf(_buffer, " alias=<i>%s</i>", alias);
					response += _buffer;
				} else {
					__ERROR("server: devlist read error\n");
				}

				// Time
				if(DevList::get_time_at_index(&time, i) != DEVLIST_OK) {
					__ERROR("server: devlist read error\n");
				}
				// mbed time is int (not unsigned int) according to mbed specification
				sprintf(_buffer, "  time=%d", (int) time);
				response += _buffer;


				// Format radku - konec
				sprintf(_buffer, "</div>");
				response += _buffer;
			}


			DevList::unlock();

			/*
			 * Sender temporarily removed from HTTP view
			 * If it will be necessary again, its better to implement it in separate item in menu
			 */
			/*
			sprintf(_buffer, "<h2>Sender status</h2>");
			response += _buffer;

			sprintf(_buffer, "<div style=\"font-family: monospace; white-space: pre;\">");
			response += _buffer;

			response += "<h3>Bus states</h3>";
			// Dratova sbernice
			sprintf(_buffer, "<div>");
			response += _buffer;
			sprintf(_buffer, "RS485  is_ready()=%s    ",  ByzanceCore::_sender->_plus_rs485->is_ready() ? "<span style=\"color:#00D000;\">true  </span>" : "<span style=\"color:#FF0000;\">false</span> ");
			response += _buffer;
			sprintf(_buffer, "plus_state=%d,  ", (int) ByzanceCore::_sender->_plus_rs485->getPlusState());
			response += _buffer;
			sprintf(_buffer, "bin_state=%d,  ", (int) ByzanceCore::_sender->_plus_rs485->getBinState());
			response += _buffer;
			sprintf(_buffer, "enum_state=%d,  ", (int) ByzanceCore::_sender->_plus_rs485->getEnumState());
			response += _buffer;
			sprintf(_buffer, "base is_ready=%s\n", ByzanceCore::_sender->_base_nrf->get_base()->is_ready() ? "<span style=\"color:#00D000;\">true  </span>" : "<span style=\"color:#FF0000;\">false</span> ");
			response += _buffer;
			sprintf(_buffer, "</div>");
			response += _buffer;

			// Bezdratova sbernice
			sprintf(_buffer, "<div>");
			response += _buffer;
			sprintf(_buffer, "NRF905 is_ready()=%s    ",  ByzanceCore::_sender->_plus_nrf->is_ready() ? "<span style=\"color:#00D000;\">true  </span>" : "<span style=\"color:#FF0000;\">false</span> ");
			response += _buffer;
			sprintf(_buffer, "plus_state=%d,  ", (int) ByzanceCore::_sender->_plus_nrf->getPlusState());
			response += _buffer;
			sprintf(_buffer, "bin_state=%d,  ", (int) ByzanceCore::_sender->_plus_nrf->getBinState());
			response += _buffer;
			sprintf(_buffer, "enum_state=%d,  ",(int) ByzanceCore::_sender->_plus_nrf->getEnumState());
			response += _buffer;
			sprintf(_buffer, "base is_ready=%s\n", ByzanceCore::_sender->_base_nrf->get_base()->is_ready() ?"<span style=\"color:#00D000;\">true  </span>" : "<span style=\"color:#FF0000;\">false</span> ");
			response += _buffer;
			sprintf(_buffer, "</div>");
			response += _buffer;


			response += "<h3>Other</h3>";
			// Vypis NRF905 adres
			sprintf(_buffer, "<div>Ioda NRF address:    Rx=0x%08X,    Tx=0x%08X</div></br>", (int) ByzanceCore::_sender->_base_nrf->get_pckt()->get_nrf_rx_address(), (int) ByzanceCore::_sender->_base_nrf->get_pckt()->get_nrf_tx_address());
			response += _buffer;


			// Vypis device manager
			switch(ByzanceCore::_sender->_dev_man.state){
				case DEV_MAN_POWER_UP:
					sprintf(alias, "<span style=\"color:#FFA500;\">POWER_UP</span>");	// oranzova FFA500
					break;
				case DEV_MAN_ACTIVE:
					sprintf(alias, "<span style=\"color:#00D000;\">ACTIVE</span>");	// zelená 006400
					break;
				case DEV_MAN_DISABLE:
					sprintf(alias, "<span style=\"color:#FF0000;\">DISABLE</span>");	// cervena FF0000
					break;
				default:
					sprintf(alias, "<span style=\"color:#FF0000;\">ERROR</span>");	// cervena FF0000
			}

			sprintf(_buffer, "<div>Device manager: state=%s (%d),   sub_state=%d,   sub_state_cntr=%d,   state_backup=%d</div>", alias, (int) ByzanceCore::_sender->_dev_man.state, (int) ByzanceCore::_sender->_dev_man.sub_state, (int) ByzanceCore::_sender->_dev_man.sub_state_cntr, (int) ByzanceCore::_sender->_dev_man.state_backup);
			response += _buffer;

			sprintf(_buffer, "</div>");
			response += _buffer;

			*/

		} else {
			response += "devlist lock error";
			PARSER_TRACE("lock error\n");
		}

		response += footer;

		return HTTP_200_OK;
#endif

	/************************************************
	 * INPUTS
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_INPUTS) == 0) {

		__INFO("requesting inputs\n");

		// vyrobim odpoved
		response += header;
		response += menu;
		response += "<h1>Inputs</h1>";
		response += "<h2>Digital</h2>";

		if (ByzanceCore::_digitalInputs->size()) {
			sprintf(_buffer, "There are %d registered digital inputs:\n", ByzanceCore::_digitalInputs->size());
			response += _buffer;
			response += "<ol start=\"0\">";
			for (unsigned int i = 0; i < ByzanceCore::_digitalInputs->size(); i++) {
				sprintf(_buffer, "<li>Digital input: %s, value = %d</li>\n", (*ByzanceCore::_digitalInputs)[i].get_name(),
						(*ByzanceCore::_digitalInputs)[i].get_last_value());
				response += _buffer;
			}
			response += "</ol>";
		} else {
			response += "There are no registered digital inputs";
		}

		response += "<h2>Analog</h2>";
		if (ByzanceCore::_analogInputs->size()) {
			sprintf(_buffer, "There are %d registered analog inputs:\n", ByzanceCore::_analogInputs->size());
			response += _buffer;
			response += "<ol start=\"0\">";
			for (unsigned int i = 0; i < ByzanceCore::_analogInputs->size(); i++) {
				sprintf(_buffer, "<li>Analog input: %s, value = %f</li>\n", (*ByzanceCore::_analogInputs)[i].getName(),
						(*ByzanceCore::_analogInputs)[i].getLastValue());
				response += _buffer;
			}
			response += "</ol>";
		} else {
			response += "There are no registered analog inputs";
		}

		response += "<h2>Message</h2>";
		if (ByzanceCore::_messageInputs->size()) {
			sprintf(_buffer, "There are %d registered message inputs:\n", ByzanceCore::_messageInputs->size());
			response += _buffer;
			response += "<ol start=\"0\">";
			for (unsigned int i = 0; i < ByzanceCore::_messageInputs->size(); i++) {
				sprintf(_buffer, "<li>Message input: %s</li>\n", (*ByzanceCore::_messageInputs)[i].get_name());
				response += _buffer;
			}
			response += "</ol>";
		} else {
			response += "There are no registered message inputs";
		}

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * MQTT
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_MQTT) == 0) {

		// vyrobim odpoved
		response += header;
		response += menu;
		response += "<h1>MQTT</h1>";

		/*
		 * overview
		 */

		// load mqtt structure
		mqtt_connection		connection;

		// outer alignment
		response+="<table><tr><td>\n";

		/*
		 * normal broker
		 */
		response += "<fieldset>";
		response += "<legend>Normal broker</legend>";

		if(MqttManager::get_connection(MQTT_NORMAL, &connection)==MQTTMGR_OK){

			response += "<form action=\"/settings-normal-broker\" method=\"post\">";

			response+="<input type=\"text\" name=\"normal_broker_hostname\" value=\"";
			response+=connection.hostname;
			response+="\"><br>";

			response+="<input type=\"text\" name=\"normal_broker_port\" value=\"";
			sprintf(_buffer, "%d", (unsigned int)connection.port);
			response+=_buffer;
			response+="\"><br>";

			response += "<br>";
			response += "<input type=\"submit\" value=\"Odeslat\">";
			response += "</form>";

		} else {
			response += "acquire error<br>\n";
		}

		response += "</fieldset><br>";

		/*
		 * backup broker
		 */
		response += "<fieldset>";
		response += "<legend>Backup broker</legend>";

		if(MqttManager::get_connection(MQTT_BACKUP, &connection)==MQTTMGR_OK){

			response += "<form action=\"/settings-backup-broker\" method=\"post\">";

			response+="<input type=\"text\" name=\"backup_broker_hostname\" value=\"";
			response+=connection.hostname;
			response+="\"><br>";

			response+="<input type=\"text\" name=\"backup_broker_port\" value=\"";
			sprintf(_buffer, "%d", (int)connection.port);
			response+=_buffer;
			response+="\"><br>";

			response += "<br>";
			response += "<input type=\"submit\" value=\"Odeslat\">";
			response += "</form>";

		} else {
			response += "acquire error<br>\n";
		}

		response += "</fieldset><br>";

		/*
		 * credentials
		 * user can only set values
		 */
		response += "<fieldset>";
		response += "<legend>Credentials</legend>";

		response += "<form action=\"/settings-credentials\" method=\"post\">";

		response+="<input type=\"password\" name=\"broker_username\"><br>";
		response+="<input type=\"password\" name=\"broker_password\"><br>";

		response += "<br>";
		response += "<input type=\"submit\" value=\"Odeslat\">";
		response += "</form>";


		response += "</fieldset><br>";

		// outer alignment
		response+="</td></tr><table>\n";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * CONFIGURATOR
	 ************************************************/
	} else if (strcmp(request, HTML_PAGE_CONFIGURATOR) == 0) {

		uint32_t	tmp32 = 0;
		int			tmpi = 0;

		char buff[64];

		__INFO("requesting configurator\n");

		// vyrobim odpoved
		response += header;
		response += menu;
		response += "<h1>Configurator</h1>";

		/*
		 * CONFIGURATION
		 */
		response+="<fieldset>";
		response+="<legend>Configuration</legend>";
		response+="<form action=\"/settings-other\" method=\"post\">";

		response+="<table>";
		response+="<tr><td>Flashflag</td><td><input type=\"checkbox\" name=\"flashflag\" ";
		int rc = Configurator::get_flashflag(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>Autobackup</td><td><input type=\"checkbox\" name=\"autobackup\" ";
		rc = Configurator::get_autobackup(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>wdenable</td><td><input type=\"checkbox\" name=\"wdenable\" ";
		rc = Configurator::get_wdenable(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>wdtime</td><td><input type=\"text\" name=\"wdtime\" value=\"";
		rc = Configurator::get_wdtime(&tmp32);
		if(rc){
			response+="error";
		}
		else{
			sprintf(_buffer, "%u", (unsigned int)tmp32);
			response+=_buffer;
		}

		response += "\"></td></tr>";

		response+="<tr><td>blreport</td><td><input type=\"checkbox\" name=\"blreport\" ";
		rc = Configurator::get_blreport(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>Netsource</td><td><input type=\"text\" name=\"netsource\" value=\"";
		rc = Configurator::get_netsource(&tmp32);
		if(rc){
			response+="error";
		}
		else{
			sprintf(_buffer, "%u", (unsigned int)tmp32);
			response+=_buffer;
		}
		response += "\"></td></tr>";

		response+="<tr><td>Configured</td><td><input type=\"checkbox\" name=\"configured\" ";
		rc = Configurator::get_configured(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>Launched</td><td><input type=\"checkbox\" name=\"launched\" ";
		rc = Configurator::get_launched(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>Alias</td><td><input type=\"text\" name=\"alias\" value=\"";
		rc = Configurator::get_alias(buff);
		if(rc){
			response+="error";
		}
		else{
			response+=buff;
		}
		response += "\"></td></tr>";

		response+="<tr><td>Webview</td><td><input type=\"checkbox\" name=\"webview\" ";
		rc = Configurator::get_webview(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>Webport</td><td><input type=\"text\" name=\"webport\" value=\"";
		rc = Configurator::get_webport(&tmp32);
		if(rc){
			response+="error";
		}
		else{
			sprintf(_buffer, "%u", (unsigned int)tmp32);
			response+=_buffer;
		}
		response += "\"></td></tr>";

		response+="<tr><td>Timeoffset</td><td><input type=\"text\" name=\"timeoffset\" value=\"";
		rc = Configurator::get_timeoffset(&tmpi);
		if(rc){
			response+="error";
		}
		else{
			sprintf(_buffer, "%d", (unsigned int)tmpi);
			response+=_buffer;
		}
		response += "\"></td></tr>";

		response+="<tr><td>Timesync</td><td><input type=\"checkbox\" name=\"timesync\"  ";
		rc = Configurator::get_timesync(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";

		response+="<tr><td>autojump</td><td><input type=\"text\" name=\"autojump\" value=\"";
		rc = Configurator::get_autojump(&tmp32);
		if(rc){
			response+="error";
		}
		else{
			sprintf(_buffer, "%u", (unsigned int)tmp32);
			response+=_buffer;
		}
		response += "\"></td></tr>";

		response+="<tr><td>Lowpanbr</td><td><input type=\"checkbox\" name=\"lowpanbr\" ";
		rc = Configurator::get_lowpanbr(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "</td></tr>";
		response += "</table>";

		response += "<br>";
		response += "<input type=\"submit\" value=\"Odeslat\">";
		response += "</form>";
		response += "</fieldset>";

		/*
		 * CONFIGURATION
		 */
		response+="<fieldset>";
		response+="<legend>Restart</legend>";
		response+="<form action=\"/settings-restart\" method=\"post\">";

		response+="Stay in bootloader <input type=\"checkbox\" name=\"restartbl\" ";
		rc = Configurator::get_restartbl(&tmp32);
		if(rc){
			response+=">error";
		}
		else if(tmp32){
			response+="checked>";
		} else{
			response+=">";
		}
		response += "<br>";

		response += "<br>";
		response += "<input type=\"submit\" value=\"Restart\">";
		response += "</form>";
		response += "</fieldset>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * SETTINGS - normal broker
	 ************************************************/
	} else if (strcmp(request, HTML_FORM_NORMAL_BROKER) == 0) {

		__INFO("handling settings-normal-broker form\n");

		// vyrobim odpoved
		response += header;
		response += menu;

		response += "<p>Normal broker is set. <a href=\"/settings\">Back to settings.</a></p>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * SETTINGS - backup broker
	 ************************************************/
	} else if (strcmp(request, HTML_FORM_BACKUP_BROKER) == 0) {

		__INFO("handling settings-backup-broker form\n");

		// vyrobim odpoved
		response += header;
		response += menu;

		response += "<p>Backup broker is set. <a href=\"/settings\">Back to settings.</a></p>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * SETTINGS - wifi form handling
	 ************************************************/
	} else if (strcmp(request, HTML_FORM_WIFI) == 0) {

		__INFO("handling settings-wifi form\n");

		// vyrobim odpoved
		response += header;
		response += menu;

		response += "<p>Wifi is set. <a href=\"/settings\">Back to settings.</a></p>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * SETTINGS - other handling
	 ************************************************/
	} else if (strcmp(request, HTML_FORM_OTHER) == 0) {

		__INFO("handling settings-other form\n");

		// vyrobim odpoved
		response += header;
		response += menu;

		response += "<p>Options are set. <a href=\"/settings\">Back to settings.</a></p>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * SETTINGS - restart
	 ************************************************/
	} else if (strcmp(request, HTML_FORM_RESTART) == 0) {

		__INFO("handling settings-restart form\n");

		// vyrobim odpoved
		response += header;
		response += menu;

		response += "<p>Restart will follow. <a href=\"/settings\">Back to settings.</a></p>";

		response += footer;

		return HTTP_200_OK;

	/************************************************
	 * E404 not found
	 ************************************************/
	} else {

		__ERROR("E404\n");

		// vyrobim odpoved
		response += header;
		response += menu;
		response += "<h1>ERROR 404 - Not Found</h1>";
		response += "Neco je spatne";
		response += footer;

		return HTTP_404_NOT_FOUND;

	}

}

void HTMLParser::show_binary(bin_component_t comp, string& buffer){

	struct_binary	tmp_bin;

	BinManager::get_bin_info(comp, &tmp_bin);

	uint8_t	tmp_progress = 0;

	buffer += "<table cellspacing='5px'>";
	sprintf(_buffer,"<tr><td>size</td><td>%u</td></tr>\n", (unsigned int)tmp_bin.size);
	buffer += _buffer;
	sprintf(_buffer,"<tr><td>crc</td><td>0x%08X</td></tr>\n", (unsigned int)tmp_bin.crc);
	buffer += _buffer;
	sprintf(_buffer,"<tr><td>build_id</td><td>%s</td></tr>\n", (char*)tmp_bin.build_id);
	buffer += _buffer;
	sprintf(_buffer, "<tr><td>version</td><td>%s</td></tr>\n", (char*)tmp_bin.version);
	buffer += _buffer;
	sprintf(_buffer,"<tr><td>timestamp</td><td>%u</td></tr>\n", (unsigned int)tmp_bin.timestamp);
	buffer += _buffer;
	sprintf(_buffer,"<tr><td>usr_version</td><td>%s</td></tr>\n", (char*)tmp_bin.usr_version);
	buffer += _buffer;
	sprintf(_buffer,"<tr><td>usr_name</td><td>%s</td></tr>\n", (char*)tmp_bin.usr_name);
	buffer += _buffer;
	buffer += "<tr><td>state</td><td>";
	switch(tmp_bin.state){
		case BINSTRUCT_STATE_INVALID:
			buffer += "<div style=\"color:red\">invalid</div>";
			break;
		case BINSTRUCT_STATE_VALID:
			buffer += "<div style=\"color:green\">valid</div>";
			break;
		case BINSTRUCT_STATE_ERASING:
			buffer += "<div style=\"color:orange\">erasing</div>";
			break;
		case BINSTRUCT_STATE_ERASED:
			buffer += "<div style=\"color:blue\">erased</div>";
			break;
		default:
			buffer += "<div style=\"color:black\">unknown</div>";
			break;
	}
	buffer += "</td></tr>\n";
	buffer += "<tr><td>progress</td><td>";
	BinManager::get_progress(comp, &tmp_progress);
	sprintf(_buffer, "%d%%", tmp_progress);
	buffer += _buffer;
	buffer += "</td></tr>";
	buffer += "<tr><td>busy</td><td>";
	if(BinManager::is_busy(comp)){
		buffer += "<div style=\"color:red\">yes</div>";
	} else {
		buffer += "<div style=\"color:green\">no</div>";
	}
	buffer += "</td></tr>";
	buffer+="</table>\n";
}
