#include "ExtMem.h"

SpiFlash			*ExtMem::_extflash;
external_spiflash	*ExtMem::_spi_settings;

char				*ExtMem::_subsector_buffer;

bool				ExtMem::_initialized;

uint16_t 			ExtMem::_secure_counter;

EXTMEM_MUTEX_DEFINITION_C;

// TODO: add bootloader - interface bootloader_print
REGISTER_DEBUG(ExtMem);
 
ExtMem_t ExtMem::init(struct external_spiflash *settings, ByzanceSpi * byz_spi){

	__TRACE("init: start\n");

	if(ExtMem::_initialized){
		__ERROR("init: reinit extmem\n");
		return EXTMEM_REINIT;
	}

	int rc = 0;

	_spi_settings = (external_spiflash*) malloc(sizeof(external_spiflash));

	// malloc failed
	if(_spi_settings==NULL){
		return EXTMEM_MALLOC_ERROR;
	}
	memcpy(_spi_settings, settings, sizeof(external_spiflash));

	_extflash = new SpiFlash(byz_spi, settings->spi_csn, settings->speed);

	_subsector_buffer = (char*)malloc(_spi_settings->subsector_size);

	// malloc failed
	if(_subsector_buffer == NULL) {
		return EXTMEM_MALLOC_ERROR;
	}

	// from now extmem is initialised
	// its necessary to enable related flag (right here) before read or write (which follows)
	_initialized = 1;

	struct_journal journal;
	rc = read_data(EXTPART_SECURE_INDEX, 0, &journal, sizeof(struct_journal));
	if(rc>0){
		__WARNING(" init: journal read failed\n");
		return EXTMEM_NOT_INITIALISED;
	}

	/*
	 * There is something in journal sector
	 * Move it to the proper place.
	 */
	rc = _handle_journal(&journal);
	if(rc>0){
		__WARNING(" init: transaction in journal found and fixed\n");
	} else if(rc<0){
		__ERROR("init: journal recovery failed = %d\n", rc);
	} else {
		__TRACE("init: journal empty\n");
	}

	return EXTMEM_OK;
}

ExtMem_t ExtMem::secure_write(uint32_t address, uint32_t offset, void * data, uint32_t size){

	EXTMEM_MUTEX_LOCK;

	// velikost dat je větší než journal
	if(size>_spi_settings->subsector_size){
		__ERROR("secure_read: requested read size is bigger than allowed journal size\n");
		EXTMEM_MUTEX_UNLOCK;
		return EXTMEM_OVERLAP;
	}

	/**********************************
	 * JOURNAL PREPARATION
	 **********************************/

	uint32_t blocks = 0;
	uint32_t i = 0;
	//uint32_t j = 0;
	uint32_t zacatek_cteni	= 0;
	uint32_t konec_cteni	= 0;
	uint32_t delka_cteni	= 0;
	uint32_t nahrazeni_zacatek	= 0;
	uint32_t precteno = 0;

	// vypočítá počet celých stránek použitých k čtení
	blocks = calculate_subsector_index(address + offset + size) - calculate_subsector_index(address+offset);

	// pokud data čouhají o nějaký kus přes celý násobek stránky, musí pracovat s celou další stránkou
	if((address+offset+size)%_spi_settings->subsector_size){
		blocks++;
	}

	zacatek_cteni = address+offset;

	// vyrobím si strukturu pro secure index
	struct_journal journal;
	journal.dest_addr	= calculate_subsector_begin(zacatek_cteni);
	journal.blocks		= blocks;

	for(i = 0; i<blocks; i++){

		// přečte požadovaný sektor
		// čtení končí jinde, než na konci aktuálního SUBSEKTORU (čtu asi víc subsektorů)
		if((address + offset + size)<=calculate_subsector_begin(zacatek_cteni)+_spi_settings->subsector_size){
			konec_cteni = address + offset + size;

		// čtu málo dat a všechny se vlezou do současného subsektoru
		} else {
			konec_cteni = calculate_subsector_begin(zacatek_cteni)+_spi_settings->subsector_size;
		}

		delka_cteni = konec_cteni - zacatek_cteni;

		//__TRACE("secure_write: zacatek=0x%08X, konec=0x%08X\n", zacatek_cteni, konec_cteni);
		//__TRACE("secure_write: ctu data 0x%08X, delka %d bytu \n", calculate_subsector_begin(zacatek_cteni), delka_cteni);

		// nactu subsektor dat

		for(uint32_t j=0; j<(_spi_settings->subsector_size/_spi_settings->page_size); j++){
			_extflash->read(calculate_subsector_begin(zacatek_cteni) + j*_spi_settings->page_size, _subsector_buffer + j*_spi_settings->page_size, _spi_settings->page_size);
		}

		nahrazeni_zacatek = zacatek_cteni - calculate_subsector_begin(zacatek_cteni);

		__TRACE("secure_write: beru pozadovane data od %d a strkam je do bloku od %d, celkem %d bytu\n", precteno, nahrazeni_zacatek, delka_cteni);

		memcpy((_subsector_buffer+nahrazeni_zacatek), (char*)((char*)data + precteno), delka_cteni);

		__TRACE("secure_write: budu zapisovat do 0x%08X, offset=0x%08X, size=%d\n", EXTPART_SECURE_JOURNAL, i*_spi_settings->subsector_size, _spi_settings->subsector_size);

		// ulozim subsektor dat do journalu
		write_data(EXTPART_SECURE_JOURNAL, i*_spi_settings->subsector_size, _subsector_buffer, _spi_settings->subsector_size);

		precteno 		+= delka_cteni;
		zacatek_cteni	+= delka_cteni;
	}

	__TRACE("secure_write: kopirovani do journalu OK\n");

	// odted jsou data v journalu platne

	// ulozim si tuto informaci do journal indexu
	write_data(EXTPART_SECURE_INDEX, 0, &journal, sizeof(struct_journal));

	__TRACE("secure_write: v journalu vemu %d bloku dat a presunu je do 0x%08X\n", journal.blocks, journal.dest_addr);

	/**********************************
	 * JOURNAL PROCESSING
	 **********************************/

	int rc = 0;

	rc = _handle_journal(&journal);
	if(rc>0){
		__WARNING(" secure_write: transaction in journal handled\n");
	} else if(rc<0){
		__ERROR("secure_write: journal recovery failed = %d\n", rc);
	} else {
		__TRACE("secure_write: journal empty\n");
	}

	//__TRACE(" -------------------------\n");
	//__TRACE("secure_write: END\n");
	//__TRACE(" -------------------------\n");

	EXTMEM_MUTEX_UNLOCK;
	return EXTMEM_OK;
}

ExtMem_t ExtMem::write_data(uint32_t address, uint32_t offset, void *data, uint32_t size, bool erase){

	EXTMEM_INIT_CHECK(EXTMEM_NOT_INITIALISED);

	EXTMEM_MUTEX_LOCK;

	__TRACE("write_data: start, addr = 0x%08X, offset = %d, size=%d\n", address, offset, size);

	// subsector = nejmenší jednotka, která jde SMAZAT, např. 4 KB
	// sector = největší jednotka, která jde smazat, např. 64 KB
	// page = nejmenší jednotka, která jde ZAPSAT, např. 256 B

	// pokud je parametr "erase" nastavený na 1 (default),
	// tato funkce si sama předmazává místo, kam bude zapisovat (pomalejší)
	// a čte/zapisuje po SUBSEKTORECH

	// pokud je parametr "erase" nastavený na 0,
	// tato funkce počítá s tím, že už místo bylo předmazáno někdy předtím
	// a zapisuje po STRÁNKÁCH (je to mnohem rychlejší)

	uint32_t i = 0;
	uint32_t j = 0;

	uint32_t zacatek_zapisu = 0;
	uint32_t konec_zapisu 	= 0;
	uint32_t delka_zapisu	= 0;

	zacatek_zapisu = address+offset;

	uint32_t nahrazeni_zacatek = 0;

	uint32_t zapsano = 0;

	// TODO: přidat kontrolu, jestli zapisuju data v rámci rozsahu adres externí flashky

	uint32_t blocks = 0;

	if(erase==1){
		// vypočítá počet celých subsektorů použitých k alokaci
		blocks = calculate_subsector_index(address + offset + size) - calculate_subsector_index(address+offset);
		// pokud data čouhají o nějaký kus přes celý násobek subsektoru, musí pracovat s celým dalším subsektorem
		if((address+offset+size)%_spi_settings->subsector_size) blocks++;
	} else {
		// vypočítá počet celých stránek použitých k alokaci
		blocks = calculate_page_index(address + offset + size) - calculate_page_index(address+offset);
		// pokud data čouhají o nějaký kus přes celý násobek stránky, musí pracovat s celou další stránkou
		if((address+offset+size)%_spi_settings->page_size) blocks++;
	}

	for(i = 0; i<blocks; i++){

		if(erase==1){
			// pokud je konec čtení jinde, než na konci SUBSEKTORU
			if((address + offset + size)<=calculate_subsector_begin(zacatek_zapisu)+_spi_settings->subsector_size){
				konec_zapisu = address + offset + size;
			} else {
				konec_zapisu = calculate_subsector_begin(zacatek_zapisu)+_spi_settings->subsector_size;
			}
		} else {
			// pokud je konec čtení jinde, než na konci STRÁNKY
			if((address + offset + size)<=calculate_page_begin(zacatek_zapisu)+_spi_settings->page_size){
				konec_zapisu = address + offset + size;
			} else {
				konec_zapisu = calculate_page_begin(zacatek_zapisu)+_spi_settings->page_size;
			}
		}

		delka_zapisu = konec_zapisu - zacatek_zapisu;

		// pokud chci subsektor přemazávat za běhu
		// musím si ho načíst do RAM po jednotlivých stránkách
		// v subsektoru v RAM se nahradí data co potřebuju zapsat
		// subsektor ve externí paměti se přemaže
		// subsektor se zapíše do externí paměti
		if(erase==1){

			/*
			 * Kontrola, jestli zapisuju cely subsektor
			 */

			char* ptr;

			// ano zapisuju -> netreba ho predtim cist
			if((zacatek_zapisu==calculate_subsector_begin(zacatek_zapisu))&&(delka_zapisu==_spi_settings->subsector_size)){
				__TRACE("write_data: read skip\n");

				ptr = (char*)data+zapsano;

			// ne, nezapisuju cely subsektor -> musim precist, co v nem bylo predtim
			// a pripravit si novy buffer k zapsani
			} else {

				//todo: nešlo by toto nějak optimalizovat?!
				// např. místo extflash přímo číst touto třídou bez cyklu?!
				for(j=0; j<(_spi_settings->subsector_size/_spi_settings->page_size); j++){
					_extflash->read(calculate_subsector_begin(zacatek_zapisu) + j*_spi_settings->page_size, _subsector_buffer + j*_spi_settings->page_size, _spi_settings->page_size);
					//__ERROR("write_data: ctu iterace j=%u\n", j);
				}

				nahrazeni_zacatek = zacatek_zapisu - calculate_subsector_begin(zacatek_zapisu);

				memcpy((_subsector_buffer+nahrazeni_zacatek), (char*)((char*)data + zapsano), delka_zapisu);

				ptr = _subsector_buffer;
			}

			_extflash->erase_subsector(zacatek_zapisu);

			for(j=0; j<(_spi_settings->subsector_size/_spi_settings->page_size); j++){

				uint32_t cnt = 0;

				// zkontroluju, jestli jsou v zapisovane strance same 0xFF
				// pokud ano, tak stranku nemusim zapisovat,
				// ptz jsem pred chvilkou zformatoval subsektor
				// coz v nem uz same 0xFF vyrobilo
				for(uint32_t k=0; k<_spi_settings->page_size; k++){
					if(ptr[j*_spi_settings->page_size+k]!=0xFF){
						// alespon jeden znak neni 0xFF -> ukoncuju hledani
						break;
					} else {
						cnt++;
					}
				}

				if(cnt==_spi_settings->page_size){
					//__TRACE("write_data: skip write\n", cnt);
				} else {
					_extflash->write(calculate_subsector_begin(zacatek_zapisu) + j*_spi_settings->page_size, ptr + j*_spi_settings->page_size, _spi_settings->page_size);
				}

			}

		// subsektor nechci přemazávat (počítám se tím, že se to udělalo mimo tuto knihovnu)
		// stačí jenom zapsat jednotlivé stránky na místa, kam patří
		} else {
			_extflash->write(zacatek_zapisu, ((char*)data + zapsano), delka_zapisu);
		}

		zapsano 		+= delka_zapisu;
		zacatek_zapisu	+= delka_zapisu;
	}

	EXTMEM_MUTEX_UNLOCK;
	return EXTMEM_OK;
}

ExtMem_t ExtMem::read_data(uint32_t address, uint32_t offset, void *data, uint32_t size){

	EXTMEM_INIT_CHECK(EXTMEM_NOT_INITIALISED);

	EXTMEM_MUTEX_LOCK;

	//__TRACE("read_data: start, addr = 0x%08X, offset = %d, size=%d\n", address, offset, size);

	// subsector = nejmenší jednotka, která jde SMAZAT, např. 4 KB
	// sector = největší jednotka, která jde smazat, např. 64 KB
	// page = nejmenší jednotka, která jde ZAPSAT, např. 256 B

	// blok = moje univerzální označení pro stránku, nebo subsektor

	uint32_t i = 0;

	uint32_t zacatek_cteni;
	uint32_t konec_cteni;
	uint32_t delka_cteni;

	uint32_t precteno = 0;

	zacatek_cteni = address+offset;

	// TODO: přidat kontrolu, jestli čtu data v rámci rozsahu adres externí flashky

	uint32_t blocks = 0;

	// vypočítá počet celých stránek použitých k čtení
	blocks = calculate_page_index(address + offset + size) - calculate_page_index(address+offset);
	// pokud data čouhají o nějaký kus přes celý násobek stránky, musí pracovat s celou další stránkou
	if((address+offset+size)%_spi_settings->page_size) blocks++;

	//__TRACE("read_data: blocks = %d\n", blocks);

	for(i = 0; i<blocks; i++){

		// pokud je konec čtení jinde, než na konci stránky
		if((address + offset + size)<=calculate_page_begin(zacatek_cteni)+_spi_settings->page_size){
			konec_cteni = address + offset + size;
		} else {
			konec_cteni = calculate_page_begin(zacatek_cteni)+_spi_settings->page_size;
		}

		delka_cteni = konec_cteni - zacatek_cteni;

		_extflash->read(zacatek_cteni, ((char*)data + precteno), delka_cteni);

		precteno += delka_cteni;
		zacatek_cteni += delka_cteni;

	}

	EXTMEM_MUTEX_UNLOCK;
	return EXTMEM_OK;
}

ExtMem_t ExtMem::erase(void){

	EXTMEM_INIT_CHECK(EXTMEM_NOT_INITIALISED);

	EXTMEM_MUTEX_LOCK;

	_extflash->erase_chip();

	EXTMEM_MUTEX_UNLOCK;
	return EXTMEM_OK;
}

ExtMem_t ExtMem::erase_subsectors(uint32_t start_index, uint32_t counter){

	EXTMEM_INIT_CHECK(EXTMEM_NOT_INITIALISED);

	EXTMEM_MUTEX_LOCK;

	uint32_t i = 0;

	for(i = 0; i<counter; i++){
		_extflash->erase_subsector((start_index + i)*_spi_settings->subsector_size);
	}

	EXTMEM_MUTEX_UNLOCK;
	return EXTMEM_OK;
}

ExtMem_t ExtMem::erase_sectors(uint32_t start_index, uint32_t counter){

	EXTMEM_INIT_CHECK(EXTMEM_NOT_INITIALISED);

	EXTMEM_MUTEX_LOCK;

	uint32_t i = 0;

	for(i = 0; i<counter; i++){
		_extflash->erase_sector((start_index + i)*_spi_settings->sector_size);
	}

	EXTMEM_MUTEX_UNLOCK;

	return EXTMEM_OK;
}

uint32_t ExtMem::calculate_subsector_index(uint32_t address){

	EXTMEM_INIT_CHECK(0);

	uint32_t index = 0;

	// zjistí, v jakém subsektoru se adresa nachází
	index = address/(_spi_settings->subsector_size);

	return index;
}

uint32_t ExtMem::calculate_sector_index(uint32_t address){

	EXTMEM_INIT_CHECK(0);

	uint32_t index = 0;

	// zjistí, v jakém subsektoru se adresa nachází
	index = address/(_spi_settings->sector_size);

	return index;
}

uint32_t ExtMem::calculate_page_index(uint32_t address){

	EXTMEM_INIT_CHECK(0);

	uint32_t index = 0;

	// zjistí, v jakém subsektoru se adresa nachází
	index = address/(_spi_settings->page_size);

	return index;
}

inline uint32_t ExtMem::calculate_subsector_begin(uint32_t address){

	EXTMEM_INIT_CHECK(0);

	return ((address/(_spi_settings->subsector_size))*_spi_settings->subsector_size);
}

inline uint32_t ExtMem::calculate_sector_begin(uint32_t address){

	EXTMEM_INIT_CHECK(0);

	return ((address/(_spi_settings->sector_size)*_spi_settings->sector_size));
}

inline uint32_t ExtMem::calculate_page_begin(uint32_t address){

	EXTMEM_INIT_CHECK(0);

	return ((address/(_spi_settings->page_size)*_spi_settings->page_size));
}

uint32_t ExtMem::get_subsector_size(void){

	EXTMEM_INIT_CHECK(0);

	return _spi_settings->subsector_size;
}

uint32_t ExtMem::get_sector_size(void){

	EXTMEM_INIT_CHECK(0);

	return _spi_settings->sector_size;
}

uint32_t ExtMem::get_page_size(void){

	EXTMEM_INIT_CHECK(0);

	return _spi_settings->page_size;
}

bool ExtMem::check_ok(void){

	EXTMEM_INIT_CHECK(1);

	EXTMEM_MUTEX_LOCK;

	char manID = 0;
	char memType = 0;
	char memCap = 0;

	_extflash->get_jedec_id(&manID, &memType, &memCap);

	if(_spi_settings->manID != manID){
		__ERROR("check_ok: expected manid=0x%02X, received=0x%02X\n", _spi_settings->manID, manID);
		EXTMEM_MUTEX_UNLOCK;
		return 1;
	}

	if(_spi_settings->memType != memType){
		__ERROR("check_ok: expected memType=0x%02X, received=0x%02X\n", _spi_settings->memType, memType);
		EXTMEM_MUTEX_UNLOCK;
		return 1;
	}

	if(_spi_settings->memCap != memCap){
		__ERROR("check_ok: expected memCap=0x%02X, received=0x%02X\n", _spi_settings->memCap, memCap);
		EXTMEM_MUTEX_UNLOCK;
		return 1;
	}

	__TRACE("check_ok: manID=0x%02X, memType=0x%02X, memCap=0x%02X\n", manID, memType, memCap);

	EXTMEM_MUTEX_UNLOCK;
	return 0;
}

int ExtMem::_handle_journal(struct_journal* journal){

	EXTMEM_MUTEX_LOCK;

	if(journal->blocks == 0xFFFFFFFF){
		__TRACE("_handle_journal: blocks unknown\n", journal->blocks);
		EXTMEM_MUTEX_UNLOCK;
		return 0;
	}

	if(journal->dest_addr == 0xFFFFFFFF){
		__TRACE("_handle_journal: dest_addr unknown\n", journal->dest_addr);
		EXTMEM_MUTEX_UNLOCK;
		return 0;
	}

	// right now we support only 2 blocks of journal
	if(journal->blocks > 2){
		__WARNING(" _handle_journal: blocks=%d too big\n", journal->blocks);
		_empty_journal(journal);
		EXTMEM_MUTEX_UNLOCK;
		return -1;
	}

	if(journal->dest_addr > EXTMEM_SIZEOF){
		__WARNING(" _handle_journal: dest_addr=0x%08X invalid\n", journal->dest_addr);
		_empty_journal(journal);
		EXTMEM_MUTEX_UNLOCK;
		return -2;
	}

	uint32_t i = 0;
	uint32_t rc = 0;

	// zkopcim data z journalu tam kam patri
	for(i = 0; i<journal->blocks; i++){

		rc = read_data(EXTPART_SECURE_JOURNAL,	i*_spi_settings->subsector_size, _subsector_buffer, _spi_settings->subsector_size);
		if(rc){
			__ERROR("_handle_journal: read error\n");
			EXTMEM_MUTEX_UNLOCK;
			return -3;
		}

		rc = write_data(journal->dest_addr, 	i*_spi_settings->subsector_size, _subsector_buffer, _spi_settings->subsector_size);
		if(rc){
			__ERROR("_handle_journal: write error\n");
			EXTMEM_MUTEX_UNLOCK;
			return -4;
		}

		__TRACE("secure_write: vzal jsem data z 0x%08X a strcil jsem je do 0x%08X\n", EXTPART_SECURE_JOURNAL + i*_spi_settings->subsector_size, journal->dest_addr + i*_spi_settings->subsector_size);

	}

	_empty_journal(journal);

	EXTMEM_MUTEX_UNLOCK;

	return 1;

}

int ExtMem::_empty_journal(struct_journal* journal){

	EXTMEM_MUTEX_LOCK;

	journal->blocks		= 0xFFFFFFFF;
	journal->dest_addr	= 0xFFFFFFFF;
	// myslím, že poslední parametr musí být sizeof(struktura), protože sizeof(pointer) bude = 4
	write_data(EXTPART_SECURE_INDEX, 0, journal, sizeof(struct_journal));

	__TRACE("_empty_journal: journal deleted\n");

	EXTMEM_MUTEX_UNLOCK;

	return 0;
}
